#include <linux/net.h>
#include <linux/socket.h>
#include <linux/inet.h>
#include <linux/tcp.h>
#include <linux/slab.h>


#include <stcp/debug.h>
#include <stcp/proto_layer.h>
#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>
#include <stcp/handshake_worker.h>
#include <stcp/stcp_protocol.h>
#include <stcp/settings.h>

#define CALL_ORIGINAL_DATA_READY    1
#define DEBUG_STACK_DUMP            0
#define HARD_BUG_ON_MAGIC_FAILURE   1

void stcp_handshake_worker(struct work_struct *work);

inline struct stcp_sock *
stcp_struct_get_st_from_socket(struct socket *sock)
{
    if (!sock || !sock->sk)
        return NULL;

    struct stcp_sock * st = sock->sk->sk_user_data;

    if (is_stcp_magic_ok(st)) {
        return st;
    }

    BUG_ON(HARD_BUG_ON_MAGIC_FAILURE);
    return NULL;
}

inline struct stcp_sock *stcp_struct_get_st_from_sk(struct sock *sk)
{
    if (!sk)
        return NULL;

    struct stcp_sock * st = sk->sk_user_data;

    u32 magic = 0;
    
    if (st) {
        magic = READ_ONCE(st->magic);
    }

    if(magic == STCP_MAGIC_DEAD)  { 
        SDBG("Has DEADBEEF");
    } else if(magic == STCP_MAGIC_ALIVE)  { 
        return st;
    } else {
        SDBG("Has not STCP_MAGIC_ALIVE");
    }

    BUG_ON(HARD_BUG_ON_MAGIC_FAILURE);
    return NULL;
}

inline struct stcp_sock *stcp_struct_get_st_from_sk_for_destroy(struct sock *sk)
{
    if (!sk)
        return NULL;

    struct stcp_sock * st = sk->sk_user_data;

    u32 magic = 0;
    
    if (st) {
        magic = READ_ONCE(st->magic);
    }

    if(magic == STCP_MAGIC_ALIVE)  { 
        return st;
    }

    if(magic == STCP_MAGIC_DEAD)  { 
        SDBG("Has DEADBEEF");
    } else {
        SDBG("Has not STCP_MAGIC_ALIVE");
    }
    return NULL;
}

void stcp_detach_from_sock(struct stcp_sock *st)
{
    if (!st) {
        return;
    }

    if (!is_stcp_magic_ok(st)) {
        SDBG("Magic check fails..");
        return;
    }

    struct sock *sk = st->sk;

    if (!sk) {
        return;
    }

    /* Palauta alkuperäiset callbackit */
    sk->sk_user_data = NULL;

    if (st->orginal_data_ready)
        sk->sk_data_ready = st->orginal_data_ready;

    if (st->orginal_state_change)
        sk->sk_state_change = st->orginal_state_change;

    SDBG("Socket[%px//%px] detached.", st, sk);
}


void stcp_rust_glue_socket_op_data_ready(struct sock *sk) {
 
    SDBG("Sock[%px] Data ready!", sk);
    if (!sk) {
        return;
    }

    struct stcp_sock *st = stcp_struct_get_st_from_sk(sk);
    if (!st) {
        SDBG("Sock[%px] Got no user data!", sk);
        return;
    }

#if CALL_ORIGINAL_DATA_READY
    SDBG("Sock[%px] Calling original data ready...", sk);
    st->orginal_data_ready(sk);
#else
    SDBG("Sock[%px] SKIPPED Call for original data ready...", sk);
#endif

    SDBG("Sock[%px] Queuing work for stcp_sock: %px", sk, st);
    stcp_queue_work_for_stcp_hanshake(st, 0, HS_PUMP_REASON_DATA_READY);
    SDBG("Sock[%px] Queued work for stcp_sock: %px", sk, st);
}

struct stcp_sock *stcp_struct_alloc_st(void)
{
    struct stcp_sock *st;

    st = kzalloc(sizeof(*st), GFP_KERNEL);

    if (!st) {
        SDBG("STCP: Structure alloc: No mem!");
        return NULL;
    }

    st->sk                 = NULL;
    st->session            = NULL;
    st->status             = 0;
    st->orginal_data_ready = NULL;

    INIT_DELAYED_WORK(&st->handshake_work, stcp_handshake_worker);

    st->the_wq = alloc_workqueue("stcp_session_wq", WQ_UNBOUND | WQ_HIGHPRI, 0);

    init_completion(&st->hs_done);
    st->hs_result = -EINPROGRESS;

    st->magic = STCP_MAGIC_ALIVE;

    SDBG("Got allocated: %px", st);
    return st;
}

void stcp_struct_free_st(struct stcp_sock *st)
{

#if DEBUG_STACK_DUMP
    // dump_stack();
#endif

    if (!st)
        return;

    u32 magic = 0;
    
    if (st) {
        magic = READ_ONCE(st->magic);
    }

    if(magic == STCP_MAGIC_DEAD)  { 
        SDBG("Has DEADBEEF");
        return;
    }

    if(magic != STCP_MAGIC_ALIVE)  { 
        SDBG("Has not STCP_MAGIC_ALIVE");
        return;
    }

    if (st->the_wq) {
        SDBG("Cancel work for %px", st);
        cancel_delayed_work_sync(&st->handshake_work);
        destroy_workqueue(st->the_wq);
        st->the_wq = NULL;
    }

    if (st->sk) {

        SDBG("Restoring data ready for sock: %px", st->sk);
        st->sk->sk_data_ready = st->orginal_data_ready;

        SDBG("Removing user data from sock: %px", st->sk);
        st->sk->sk_user_data = NULL;

        int tcpStateOK = (st->sk->sk_state != TCP_CLOSE ) &&
                         (st->sk->sk_state != TCP_LISTEN);

        int fatal = (st->status & STCP_STATUS_SOCKET_FATAL_ERROR) > 0;
        SDBG("tcpStateOK: %d, Fatal: %d", tcpStateOK, fatal);

        
        if (st->status & STCP_STATUS_SOCKET_LISTENING) {
            SDBG("Free: listening socket -> never mark sk_err");
        } else if (tcpStateOK && fatal) {
            SDBG("Got fatal error => Marking protocol error to sock: %px", st->sk);
            st->sk->sk_err = EPROTO;
            sk_error_report(st->sk);
        }
    }

    if (st->session) {
        SDBG("Freeing session: %px", st->session);
        rust_exported_session_destroy(st->session);
        st->session = NULL;
    }

    // Invalidate memory
    SDBG("Poisoning %px...", st);
    memset(st, 0xA5, sizeof(*st));
    st->magic = STCP_MAGIC_DEAD;
    SDBG("Freeing poisoned: %px", st);

    SDBG("Freeing: %px", st);
    kfree(st);
}


int stcp_struct_attach_st_to_socket(
    struct stcp_sock *st, struct socket *sock)
{

    if (!st || !sock || !sock->sk) {
        return -EINVAL;
    }

    return stcp_struct_attach_st_to_sk(st, sock->sk);
}

int stcp_struct_attach_st_to_sk(
    struct stcp_sock *st, struct sock *sk)
{

    if (!st || !sk) {
        return -EINVAL;
    }

    if (sk->sk_user_data) {
        return 0;
    }

    /* atominen attach: jos joku ehti ensin, vapauta meidän st */
    if (cmpxchg(&sk->sk_user_data, NULL, st) != NULL) {
        stcp_struct_free_st(st);
        return -EBADF;
    }

    st->sk = sk;

    /* data_ready hookki: tee tämä vasta kun varmasti haluat */
    st->orginal_data_ready = sk->sk_data_ready;
    sk->sk_data_ready = stcp_rust_glue_socket_op_data_ready;

    SDBG("Attached stcp_sock %px to sock %px", st, sk);
    return 0;
}

inline struct stcp_sock *stcp_struct_get_or_alloc_st_from_sk(struct sock *sk)
{
    struct stcp_sock *st;

    if (!sk)
        return NULL;

    st = (struct stcp_sock *)sk->sk_user_data;
    if (st)
        return st;

    st = stcp_struct_alloc_st();
    if (!st)
        return NULL;

    stcp_struct_attach_st_to_sk(st, sk);
    return st;
}

inline struct stcp_sock *stcp_struct_get_or_alloc_st_from_socket(struct socket *sock)
{

    if (!sock || !sock->sk) {
        return NULL;
    }

    struct stcp_sock *st = stcp_struct_get_or_alloc_st_from_sk(sock->sk);
    return st;
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

