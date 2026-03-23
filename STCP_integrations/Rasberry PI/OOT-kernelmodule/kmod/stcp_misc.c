// stcp_misc.c

#include <linux/slab.h>
#include <linux/types.h>
#include <stcp/rust_alloc.h>
#include <linux/kernel.h>
#include <linux/net.h>
#include <linux/tcp.h>
#include <net/sock.h>
#include <net/inet_common.h>
#include <net/tcp.h>

#include <stcp/debug.h>
#include <stcp/rust_exported_functions.h>
#include <stcp/stcp_proto_ops.h>
#include <stcp/handshake_worker.h>
#include <stcp/stcp_misc.h>
#include <stcp/settings.h>
#include <stcp/proto_layer.h>
#include <stcp/stcp_protocol.h>
#include <stcp/stcp_socket_struct.h>


#define ALLOC_ZERO(mVar) \
    mVar = stcp_rust_kernel_alloc(sizeof(*mVar))



void stcp_struct_session_destroy_request(struct stcp_sock *st, int reason)
{
    if (!st) return;
    SDBG("Got st: %px", st);
    /* idempotent */
    if (test_and_set_bit(STCP_FLAG_SOCKET_DESTROY_QUEUED_BIT, &st->flags))
        return;

    SDBG("Queuing destroy of %px ...", st);
    st = stcp_get_st_ref_from_sk(st->sk);
    /* jos ollaan IRQ/BH:ssa -> schedule_work, älä tee mitään muuta */
    queue_work(st->the_wq_session_destroy, &st->session_destroy_work);
}

void stcp_struct_destroy_workfn(struct work_struct *work)
{

    SDBG("Starting destroy of %px ...", work);
    struct stcp_sock *st = container_of(work, struct stcp_sock, session_destroy_work);
    DEBUG_INCOMING_STCP_STATUS(st);

    if(st && test_and_set_bit(STCP_FLAG_SOCKET_DESTROY_DONE_BIT, &st->flags)) {
        SDBG("Stcp destriy already done for sk: %px, st: %px", st->sk, st);
        return;
    }

    if (!st) {
        SDBG("DestroyWorker[%px//%px]: FAIL: No st!", work, st);
        return;
    } else {
        SDBG("Session to destroy %px ...", st->session);
        if (st->session) {
            rust_exported_session_destroy(st->session);
            st->session = NULL;
        }
        clear_bit(STCP_FLAG_SOCKET_DESTROY_QUEUED_BIT, &st->flags);
        // ÄLÄ koske muuhun!
    }

    stcp_state_put_st(st);
    SDBG("Done destroy of %px ...", st);
    /* jos käytät refcountia: stcp_put(st); */
}


void stcp_misc_ecdh_key_free(void *pFreePtr)
{
    if (pFreePtr) {
        kfree(pFreePtr);
    }
}

void *stcp_misc_ecdh_public_key_new(void)
{
    struct stcp_ecdh_public_key *pk;

    ALLOC_ZERO(pk);

    return pk;
}

void *stcp_misc_ecdh_shared_key_new(void)
{
    struct stcp_ecdh_shared_key *sk;

    ALLOC_ZERO(sk);

    return sk;
}

void *stcp_misc_ecdh_private_key_new(void)
{
    struct stcp_ecdh_private_key *pk;

    ALLOC_ZERO(pk);

    return pk;
}

int stcp_misc_ecdh_private_key_size(void) { return sizeof(struct stcp_ecdh_private_key); }
int stcp_misc_ecdh_public_key_size(void)  { return sizeof(struct stcp_ecdh_public_key); }
int stcp_misc_ecdh_shared_key_size(void)  { return sizeof(struct stcp_ecdh_shared_key); }

int is_handshake_in_progress(struct sock *sk) {
    struct stcp_sock* st = stcp_struct_get_st_from_sk(sk);
    if (!st) {
        return -EINVAL;
    }

    int isPending  = test_bit(STCP_FLAG_HS_PENDING_BIT, &st->flags);
    int isInQueue  = test_bit(STCP_FLAG_HS_QUEUED_BIT, &st->flags);
    int isFailed   = test_bit(STCP_FLAG_HS_FAILED_BIT, &st->flags);
    int isComplete = test_bit(STCP_FLAG_HS_COMPLETE_BIT, &st->flags);

    SDBG("HS/in progress: Pending: %d, isInQueue: %d, Completed: %d, Failed: %d",
        isPending, isInQueue, isComplete, isFailed);

    int isStarted = isPending | isInQueue;

    if (isFailed) {
        return -EINVAL;
    }

    if (isStarted) {
        if (!isComplete) {
            // In progress
            return 1;
        }
    }
    return 0;
}

inline int stcp_state_is_handshake_complete(struct stcp_sock *st)
{
    if (!st) return -EAGAIN;
    return test_bit(STCP_FLAG_HS_COMPLETE_BIT, &st->flags);
}

/*
   KUTSUTAAN VAIN KUN HANSHAKE ON TEHTY!
*/
inline void stcp_state_restore_data_ready_hook(struct stcp_sock *st)
{
    struct sock *sk;
    void (*orig)(struct sock *sk);

    if (!st)
        return;

    sk   = READ_ONCE(st->sk);
    orig = READ_ONCE(st->orginal_data_ready);

    if (!sk || !orig)
        return;
    
    DEBUG_INCOMING_STCP_STATUS(st);
    SDBG("data_ready: sk=%px state=%d sk_data_ready=%ps orig=%ps proto=%px",
        sk, sk->sk_state, sk->sk_data_ready, st->orginal_data_ready, sk->sk_prot);


    if (test_bit(STCP_FLAG_SOCKET_DATA_READY_RESTORED_BIT, &st->flags)) {
        SDBG("Already restored..");
        return;
    }

    bh_lock_sock(sk);

    if (test_bit(STCP_FLAG_SOCKET_DATA_READY_RESTORED_BIT, &st->flags)) {
        SDBG("Already restored/inner..");
        return;
    }

    SDBG("Restoring socket %px data_ready -> %ps (st=%px)", sk, orig, st);
    WRITE_ONCE(sk->sk_data_ready, orig);

    set_bit(STCP_FLAG_SOCKET_DATA_READY_RESTORED_BIT, &st->flags);
    bh_unlock_sock(sk);
}

inline int stcp_state_try_start_handshake(struct stcp_sock *st, int server_side) {
    if (st) {

        if (test_and_set_bit(STCP_FLAG_HS_STARTED_BIT, &st->flags)) {
            return 0; // Oli jo
        } 

        // Aloita
        SDBG("Starting HS for connection: %px", st);
        stcp_handshake_start(st, server_side);
        return 1;
    }
    return -EINVAL;
}

int is_stcp_magic_ok(struct stcp_sock* st) 
{
    if (!st) return 0;

    /* READ_ONCE estää compiler-temppuilun */
    u32 m = READ_ONCE(st->magic);

#if STCP_IS_MAGIC_CHK_IF_DEAD 
    if(m == STCP_MAGIC_DEAD)  { BUG(); }
#endif

#if STCP_IS_MAGIC_CHK_IF_NOT_ALIVE 
    if(m != STCP_MAGIC_ALIVE) { BUG(); }
#endif

    return m == STCP_MAGIC_ALIVE;
}


int stcp_proto_setup(void)
{

#if USE_OWN_PROT_OPTS
    stcp_prot = tcp_prot;

    orginal_tcp_sendmsg  = tcp_prot.sendmsg;
    orginal_tcp_recvmsg  = tcp_prot.recvmsg;
    orginal_tcp_connect  = tcp_prot.connect;
    orginal_tcp_accept   = tcp_prot.accept;   
    
    STCP_SET_PROTO_NAME(&stcp_prot);
    stcp_prot.owner   = THIS_MODULE;

    stcp_prot.init    = stcp_init_sock;
    stcp_prot.connect = stcp_connect;
    stcp_prot.accept  = stcp_accept;
    stcp_prot.destroy = stcp_destroy;
    stcp_prot.sendmsg = stcp_sendmsg;
    stcp_prot.recvmsg = stcp_recvmsg;

#endif // USE_OWN_PROT_OPTS

    return 0;
}

