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
#include <stcp/stcp_misc.h>
#include <stcp/settings.h>
#include <stcp/state.h>
#include <linux/rcupdate.h>

#define CALL_ORIGINAL_DATA_READY           1
#define DEBUG_STACK_DUMP                   0

// verbose flags, just to help remembreing
#define FREE_MEM        0
#define DO_NOT_FREE_MEM 1

#define DO_NOT_FREE_ON_MAGIC_FAILURE       DO_NOT_FREE_MEM


extern struct proto_ops stcp_stream_ops;
extern struct proto stcp_prot;

void stcp_handshake_worker(struct work_struct *work);

inline struct stcp_sock *stcp_struct_get_st_from_socket(struct socket *sock)
{
    struct sock *sk;

    if (!sock)
        return NULL;

    sk = READ_ONCE(sock->sk);
    if (!sk)
        return NULL;

    return stcp_struct_get_st_from_sk(sk);
}


inline struct stcp_sock *stcp_struct_get_st_from_sk(struct sock *sk)
{
    struct stcp_sock *st;
    u32 magic;

    if (!sk)
        return NULL;

    st = stcp_get_st_ref_from_sk(sk);

    if (!st) return NULL;

    magic = READ_ONCE(st->magic);

    if (magic == STCP_MAGIC_ALIVE) {
        return st;
    }

    if (magic == STCP_MAGIC_DEAD) {
        SDBG("Has DEADBEEF");
    } else {
        SDBG("Has not STCP_MAGIC_ALIVE");
    }

    WARN_ON_ONCE(WARN_ON_MAGIC_FAILURE);
    return NULL;
}

inline struct stcp_sock *stcp_struct_get_st_from_sk_for_destroy(struct sock *sk)
{
    return stcp_struct_get_st_from_sk(sk);
}

inline struct stcp_sock *stcp_detach_once(struct sock *sk)
{
    if (!sk)
        return NULL;

    /* irrota tasan kerran */
    RCU_INIT_POINTER(sk->sk_user_data, NULL);
    return xchg((struct stcp_sock **)&sk->sk_user_data, NULL);
}

// STCP Struct version
void stcp_detach_from_st(struct stcp_sock *st)
{
    if (!st)
        return;

    if (!is_stcp_magic_ok(st)) {
        SDBG("Magic check fails..");
        return;
    }

    if (test_and_set_bit(STCP_FLAG_SOCKET_DETACHED_BIT, &st->flags)) {
        SDBG("Already done...");
        return;
    }


    struct sock *sk = READ_ONCE(st->sk);

    if (!sk)
        return;

    write_lock_bh(&sk->sk_callback_lock);
    
    /* Palauta callbackit (vain jos ne on tallessa) */
    if (READ_ONCE(st->orginal_data_ready))
        WRITE_ONCE(sk->sk_data_ready, READ_ONCE(st->orginal_data_ready));

    if (READ_ONCE(st->orginal_state_change))
        WRITE_ONCE(sk->sk_state_change, READ_ONCE(st->orginal_state_change));

    /* estä myöhempi käyttö st->sk:n kautta */
    rcu_assign_pointer(sk->sk_user_data, NULL);
    WRITE_ONCE(st->sk, NULL);
    write_unlock_bh(&sk->sk_callback_lock);

    /* jos haluat, voit merkitä lipun nyt (turvallista nyt) */
    set_bit(STCP_FLAG_SOCKET_DETACHED_BIT, &st->flags);

    SDBG("Socket detach work requested.. st=%px sk=%px", st, sk);

    // RCU free hoituu destrutorissa

    /* Tässä kohtaa: älä freeä st:tä vielä ellei tämä on lopullinen destroy-polku */
    stcp_struct_session_destroy_request(st, REASON_DESTROY_FROM_OK_CONTEXT);
}

// Sock struct version
void stcp_detach_from_sk(struct sock *sk)
{
    struct stcp_sock *st = NULL;

    st = xchg((struct stcp_sock **)&sk->sk_user_data, NULL);
    if (!st)
        return;

    if (!is_stcp_magic_ok(st)) {
        SDBG("Magic check fails..");
        return;
    }

    write_lock_bh(&sk->sk_callback_lock);

    /* Palauta callbackit (vain jos ne on tallessa) */
    if (READ_ONCE(st->orginal_data_ready))
        WRITE_ONCE(sk->sk_data_ready, READ_ONCE(st->orginal_data_ready));

    if (READ_ONCE(st->orginal_state_change))
        WRITE_ONCE(sk->sk_state_change, READ_ONCE(st->orginal_state_change));

    /* estä myöhempi käyttö st->sk:n kautta */
    rcu_assign_pointer(sk->sk_user_data, NULL);
    WRITE_ONCE(st->sk, NULL);
    write_unlock_bh(&sk->sk_callback_lock);

    /* jos haluat, voit merkitä lipun nyt (turvallista nyt) */
    set_bit(STCP_FLAG_SOCKET_DETACHED_BIT, &st->flags);

    SDBG("Socket detached st=%px sk=%px", st, sk);

    /* Tässä kohtaa: älä freeä st:tä vielä ellei tämä on lopullinen destroy-polku */
    stcp_struct_session_destroy_request(st, REASON_DESTROY_FROM_OK_CONTEXT);
}

void stcp_rust_glue_socket_op_data_ready(struct sock *sk)
{
    struct stcp_sock *st = NULL;
    void (*orig)(struct sock *sk) = NULL;

    if (!sk)
        return;

    rcu_read_lock();
    st = rcu_dereference(sk->sk_user_data);
    if (st)
        orig = READ_ONCE(st->orginal_data_ready);
    rcu_read_unlock();

    /* Jos ei st:tä tai detached -> delegoi orig jos mahdollista */
    if (!st || test_bit(STCP_FLAG_SOCKET_DETACHED_BIT, &st->flags)) {
        if (orig && orig != stcp_rust_glue_socket_op_data_ready)
            orig(sk);
        return;
    }

    /* älä ikinä LISTEN */
    if (READ_ONCE(sk->sk_state) == TCP_LISTEN) {
        if (orig && orig != stcp_rust_glue_socket_op_data_ready)
            orig(sk);
        return;
    }

    /* varmista magic ennen kuin queueat */
    if (!is_stcp_magic_ok(st)) {
        if (orig && orig != stcp_rust_glue_socket_op_data_ready)
            orig(sk);
        return;
    }

    /* Jos handshake jo valmis, ei queuea */
    if (stcp_state_is_handshake_complete(st) > 0) {
        if (orig && orig != stcp_rust_glue_socket_op_data_ready)
            orig(sk);
        return;
    }

    /* Re-entrancy guard: estää data_ready loopin */
    if (test_and_set_bit(STCP_FLAG_SOCKET_IN_DATA_READY_BIT, &st->flags)) {
        if (orig && orig != stcp_rust_glue_socket_op_data_ready)
            orig(sk);
        return;
    }

    /* HERÄTE: queue work, ei muuta */
    stcp_rust_queue_work_for_stcp_hanshake(st, 0, HS_PUMP_REASON_DATA_READY);

    clear_bit(STCP_FLAG_SOCKET_IN_DATA_READY_BIT, &st->flags);

    /* delegoi orig lopuksi */
    if (orig && orig != stcp_rust_glue_socket_op_data_ready)
        orig(sk);
}

inline void stcp_struct_put_st(struct stcp_sock *st)
{
    if (!st)
        return;

    if (refcount_dec_and_test(&st->refcnt)) {
        /* tässä vaiheessa kaiken “finalize/cleanup” pitää olla jo tehty */
        call_rcu(&st->rcu, stcp_struct_free_rcu_cb);
    }
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
    st->flags              = 0;
    st->pump_counter       = 0;
    st->handshake_status   = STCP_HS_STATUS_NONE;
    st->orginal_data_ready = NULL;

    /* handshake */
    INIT_DELAYED_WORK(&st->handshake_work, stcp_handshake_worker);
    st->the_wq = alloc_workqueue("stcp_session_wq", WQ_UNBOUND | WQ_HIGHPRI | WQ_MEM_RECLAIM, 0);

    /* destroy */
    INIT_WORK(&st->session_destroy_work, stcp_struct_destroy_workfn);
    st->the_wq_session_destroy = alloc_workqueue("stcp_session_destroy_wq", WQ_UNBOUND | WQ_MEM_RECLAIM, 0);

    init_completion(&st->hs_done);
    st->hs_result = -EINPROGRESS;

    st->magic = STCP_MAGIC_ALIVE;

    refcount_set(&st->refcnt, 1);

    SDBG("Got allocated: %px", st);
    return st;
}

void stcp_struct_free_rcu_cb(struct rcu_head *h)
{
    struct stcp_sock *st = container_of(h, struct stcp_sock, rcu);

    SDBG("Poisoning %px...", st);
    memset(st, 0xA5, sizeof(*st));
    SDBG("Freeing poisoned: %px", st);

    SDBG("Freeing: %px", st);
    kfree(st);
    st = NULL;
}

void stcp_struct_free_st(struct stcp_sock *st)
{
    if (!st)
        return;

    if (stcp_state_try_acquire_free(st) != 1) {
        WARN_ON(WARN_ON_DOUBLE_FREE);
        return;
    }

    set_bit(STCP_FLAG_HS_EXIT_MODE_BIT, &st->flags);

    cancel_delayed_work_sync(&st->handshake_work);
    cancel_work_sync(&st->session_destroy_work);

    if (st->the_wq) { 
        SDBG("Destroying WQ: %px", st->the_wq);
        destroy_workqueue(st->the_wq);
        st->the_wq = NULL;
    }
    if (st->the_wq_session_destroy) { 
        SDBG("Destroying WQ for destroy: %px", st->the_wq_session_destroy);
        destroy_workqueue(st->the_wq_session_destroy); 
        st->the_wq_session_destroy = NULL;
    }

/*
    if (st->session) {
        SDBG("Freeing session: %px", st->session);
        rust_exported_session_destroy(st->session);
        st->session = NULL;
    }
*/
    if (st->sk) {
        stcp_detach_from_sk(st->sk);        
    }


    SDBG("Marking magic (ST:%px) as DEAD", st);
    WRITE_ONCE(st->magic, STCP_MAGIC_DEAD);

    call_rcu(&st->rcu, stcp_struct_free_rcu_cb);
}

int stcp_struct_attach_st_to_socket(
    struct stcp_sock *st, struct socket *sock)
{

    if (!st || !sock || !sock->sk) {
        return -EINVAL;
    }

    return stcp_struct_attach_st_to_sk(st, sock->sk);
}

int stcp_struct_attach_st_to_sk(struct stcp_sock *st, struct sock *sk)
{
    void *cur;

    if (!st || !sk)
        return -EINVAL;

    if (stcp_state_is_listening_socket(sk)) {
        SDBG("Socket %px is listening, not attaching", sk);
        return 0;
    }

    write_lock_bh(&sk->sk_callback_lock);

    cur = rcu_dereference_protected(sk->sk_user_data,
                                    lockdep_is_held(&sk->sk_callback_lock));
    if (cur) {
        write_unlock_bh(&sk->sk_callback_lock);
        return 0;
    }

    /* tallenna orig ennen overridea */
    st->orginal_data_ready = sk->sk_data_ready;
    sk->sk_data_ready = stcp_rust_glue_socket_op_data_ready;

    /* publish st */
    rcu_assign_pointer(sk->sk_user_data, st);
    WRITE_ONCE(st->sk, sk);

    write_unlock_bh(&sk->sk_callback_lock);

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
