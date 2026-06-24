#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/net.h>
#include <linux/errno.h>
#include <linux/jiffies.h>
#include <linux/sched/signal.h>
#include <net/tcp.h>

#include <net/sock.h>
#include <net/inet_common.h>
#include <net/inet_connection_sock.h>
#include <net/protocol.h>
#include <net/tcp.h>
#include <linux/inet.h>

#include <stcp/debug.h>
#include <stcp/proto_layer.h>   // Rust proto_ops API

#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>
#include <stcp/proto_operations.h>
#include <stcp/handshake_worker.h>
#include <stcp/state.h>


inline int stcp_state_is_listening_socket(struct sock *sk) {
    if (sk) {
        return READ_ONCE(sk->sk_state) == TCP_LISTEN;
    }
    return 0;
}

inline struct stcp_sock *stcp_get_st_ref_from_sk(struct sock *sk)
{
    struct stcp_sock *st;

    rcu_read_lock();
    st = rcu_dereference(sk->sk_user_data);
    if (st && READ_ONCE(st->magic) == STCP_MAGIC_ALIVE &&
        refcount_inc_not_zero(&st->refcnt)) {
        rcu_read_unlock();
        return st;
    }
    rcu_read_unlock();
    return NULL;
}

inline void stcp_state_put_st(struct stcp_sock *st)
{
    if (refcount_dec_and_test(&st->refcnt))
        kfree_rcu(st, rcu);
}


inline int stcp_state_try_acquire_free(struct stcp_sock *st)
{
    if (!st)
        return 0;
    
    /* true = minä hoidan, false = joku muu hoitaa / jo hoidettu */
    return !test_and_set_bit(STCP_FLAG_SOCKET_FREE_QUEUED_BIT, &st->flags);
}

#include <linux/completion.h>

inline int stcp_wait_for_flag_or_timeout(struct stcp_sock *st,
                                                unsigned long the_bit,
                                                unsigned int timeout_ms,
                                                bool nonblock)
{

    if (!st)
        return -EINVAL;

    if (test_bit(the_bit, &st->flags))
        return 0;

    if (nonblock)
        return -EAGAIN;

    {
        unsigned long deadline = jiffies + msecs_to_jiffies(timeout_ms);
        for (;;) {
            if (test_bit(the_bit, &st->flags))
                return 0;
            if (signal_pending(current))
                return -ERESTARTSYS;
            if (time_after(jiffies, deadline))
                return -ETIMEDOUT;
            usleep_range(1000, 2000);
        }
    }
}

int stcp_state_wait_for_handshake_or_timeout(struct stcp_sock *st,
                                             unsigned int bit_complete,
                                             unsigned int timeout_ms,
                                             bool nonblock)
{
    long t;

    if (!st)
        return -EINVAL;

    /* fail fast */
    if (test_bit(STCP_FLAG_HS_FAILED_BIT, &st->flags)) 
        return -EPROTO;

    if (test_bit(bit_complete, &st->flags)) {

        return 0;
    }

    if (nonblock)
        return -EAGAIN;

    t = wait_for_completion_interruptible_timeout(&st->hs_done,
                                                  msecs_to_jiffies(timeout_ms));
    if (t < 0)
        return (int)t;

    if (t == 0)
        return -ETIMEDOUT;

    /* woke up -> decide based on flags */
    if (test_bit(STCP_FLAG_HS_FAILED_BIT, &st->flags))
        return -EPROTO;

    if (test_bit(bit_complete, &st->flags))
        return 0;

    /*
     * Heräsi mutta kumpikaan ei ole asetettu:
     * -> joku complete kutsuttu liian aikaisin tai väärä completion.
     * Blocking-case: tämä on oikeasti "protocol bug", palauta EPROTO,
     * ettei mennä send/recv looppeihin.
     */
    return -EPROTO;
}

// hanshake helppereitä
inline void stcp_state_hanshake_reset(struct stcp_sock *st)
{
    if (!st)
        return;

    clear_bit(STCP_FLAG_HS_PENDING_BIT,   &st->flags);
    clear_bit(STCP_FLAG_HS_QUEUED_BIT,    &st->flags);
    clear_bit(STCP_FLAG_HS_COMPLETE_BIT,  &st->flags);
    clear_bit(STCP_FLAG_HS_FAILED_BIT,    &st->flags);
    clear_bit(STCP_FLAG_HS_EXIT_MODE_BIT, &st->flags);

    /* HS_STARTED jää *päälle* kun handshake on käynnissä / tehty.
       Se on se “only once” -guard. Älä clear tätä resetissä. */

    st->hs_result = 0;
    reinit_completion(&st->hs_done);
}

inline bool stcp_state_hanshake_is_complete(struct stcp_sock *st)
{
    DEBUG_INCOMING_STCP_STATUS(st);
    return test_bit(STCP_FLAG_HS_COMPLETE_BIT, &st->flags);
}

inline bool stcp_state_hanshake_is_failed(struct stcp_sock *st)
{
    DEBUG_INCOMING_STCP_STATUS(st);
    return test_bit(STCP_FLAG_HS_FAILED_BIT, &st->flags) ||
           test_bit(STCP_FLAG_HS_EXIT_MODE_BIT, &st->flags);
}

inline void stcp_state_hanshake_mark_complete(struct stcp_sock *st)
{
    DEBUG_INCOMING_STCP_STATUS(st);
    /* idempotent */
    set_bit(STCP_FLAG_HS_COMPLETE_BIT, &st->flags);
    clear_bit(STCP_FLAG_HS_PENDING_BIT, &st->flags);
    clear_bit(STCP_FLAG_HS_QUEUED_BIT, &st->flags);
    clear_bit(STCP_FLAG_HS_EXIT_MODE_BIT, &st->flags);

    st->hs_result = 0;
    complete_all(&st->hs_done);
}

inline void stcp_state_hanshake_mark_failed(struct stcp_sock *st, int err)
{
    DEBUG_INCOMING_STCP_STATUS(st);
    set_bit(STCP_FLAG_HS_FAILED_BIT, &st->flags);
    set_bit(STCP_FLAG_HS_EXIT_MODE_BIT, &st->flags);
    clear_bit(STCP_FLAG_HS_PENDING_BIT, &st->flags);
    clear_bit(STCP_FLAG_HS_QUEUED_BIT, &st->flags);

    st->hs_result = err < 0 ? err : -EPROTO;
    complete_all(&st->hs_done);
}

inline int stcp_state_wait_connection_established(struct sock *sk,
                                                  unsigned int tcp_wait_timeout_ms,
                                                  unsigned int stcp_wait_timeout_ms,
                                                  int msg_flags) 
{
    if (!sk) {
        SDBG("No SK!");
        return -EINVAL;
    }

    struct stcp_sock *st = stcp_struct_get_st_from_sk(sk);
    if (!st) {
        SDBG("No ST!");
        return -EINVAL;
    }
    DEBUG_INCOMING_STCP_STATUS(st);

    SDBG("Starting HS if not started...");
    if (!test_and_set_bit(STCP_FLAG_HS_STARTED_BIT, &st->flags)) {
        SDBG("Starting HS...");
        stcp_handshake_start(st, 
            st->is_server ? 
                START_HANDSHAKE_SIDE_SERVER :
                START_HANDSHAKE_SIDE_CLIENT);
    }
    
    stcp_log_ctx("waiting for connection", st, sk);

    bool nonblock = stcp_state_is_nonblock_sock(sk, msg_flags);

    SDBG("Starting waiting TCP, timeout %u ms", tcp_wait_timeout_ms);
    int ret = stcp_state_wait_tcp_established(sk, tcp_wait_timeout_ms, nonblock);
    if (ret < 0) {
        SDBG("Got error when waiting TCP, ret: %d", ret);
        return ret;
    }

    SDBG("Starting waiting STCP, timeout %u ms", stcp_wait_timeout_ms);
    int isHSDone = stcp_state_wait_for_handshake_or_timeout(
        st, STCP_FLAG_HS_COMPLETE_BIT, stcp_wait_timeout_ms, nonblock);

    if (isHSDone == -EAGAIN || isHSDone == -ETIMEDOUT) {
        ret = nonblock ? -EINPROGRESS : -ETIMEDOUT;
        SDBG("STCP Connection ret err, %d", ret);
        return ret;
    }

    SDBG("STCP Connection final return, ret %d", isHSDone);
    return isHSDone;
}

inline bool stcp_state_is_nonblock_sock(struct sock *sk, int msg_flags)
{
    if (msg_flags & MSG_DONTWAIT) return true;
    if (sk->sk_socket && sk->sk_socket->file &&
        (sk->sk_socket->file->f_flags & O_NONBLOCK))
        return true;
    return false;
}

inline int stcp_state_wait_tcp_established(struct sock *sk,
                                            unsigned int timeout_ms,
                                            bool nonblock)
{
    long tmo;

    if (sk->sk_state == TCP_ESTABLISHED)
        return 0;

    if (nonblock)
        return -EAGAIN;

    tmo = msecs_to_jiffies(timeout_ms);

    /* odota että state vaihtuu, tai tulee error/signal */
    tmo = wait_event_interruptible_timeout(*sk_sleep(sk),
            sk->sk_state == TCP_ESTABLISHED ||
            sk->sk_err ||
            sk->sk_state == TCP_CLOSE,
            tmo);

    if (tmo < 0)
        return tmo; /* -ERESTARTSYS */

    if (tmo == 0)
        return -ETIMEDOUT;

    if (sk->sk_err)
        return -sk->sk_err;

    if (sk->sk_state != TCP_ESTABLISHED)
        return -ECONNRESET;

    return 0;
}
