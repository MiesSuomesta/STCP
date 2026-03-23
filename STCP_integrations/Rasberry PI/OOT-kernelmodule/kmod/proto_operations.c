#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/net.h>
#include <linux/errno.h>
#include <linux/jiffies.h>
#include <linux/fcntl.h>

#include <net/sock.h>
#include <net/inet_common.h>
#include <net/inet_connection_sock.h>
#include <net/protocol.h>
#include <net/tcp.h>
#include <linux/inet.h>

#include <stcp/debug.h>
#include <stcp/proto_layer.h>

#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>
#include <stcp/proto_operations.h>
#include <stcp/handshake_worker.h>
#include <stcp/stcp_tcp_low_level_operations.h>
#include <stcp/stcp_proto_ops_helpers.h>

/* ============================================================ */
/*
 * Handshake policy:
 *  - HS is started ONLY from connect()/accept() path.
 *  - sendmsg/recvmsg NEVER schedules HS work. They only wait / return EAGAIN.
 *  - If HS fails -> socket must drop (EPROTO).
 *
 * Flags used (must exist in your headers):
 *  STCP_FLAG_HS_STARTED_BIT
 *  STCP_FLAG_HS_COMPLETE_BIT
 *  STCP_FLAG_HS_FAILED_BIT
 *  STCP_FLAG_HS_EXIT_MODE_BIT
 *  STCP_FLAG_INTERNAL_IO_BIT
 *
 * Optional:
 *  STCP_FLAG_ROLE_SERVER_BIT (if you store role in flags)
 */

/* ---------- helpers ---------- */

static inline bool stcp_is_nonblock_send(const struct sock *sk, const struct msghdr *msg)
{
    if (!sk || !msg) return false;

    if (msg->msg_flags & MSG_DONTWAIT)
        return true;

    if (sk->sk_socket && sk->sk_socket->file &&
        (sk->sk_socket->file->f_flags & O_NONBLOCK))
        return true;

    return false;
}

/*
 * Wait until handshake is complete OR failed, without starting it.
 * Returns:
 *   0        -> HS complete
 *   -EAGAIN  -> HS not complete (nonblock / not started / still pending)
 *   -ETIMEDOUT / -ERESTARTSYS -> wait errors
 *   -EPROTO  -> HS failed (or st->hs_result < 0)
 *   -ESHUTDOWN -> exit mode
 */
static inline int stcp_wait_hs_or_eagain(struct stcp_sock *st,
                                        bool nonblock,
                                        unsigned int timeout_ms)
{
    long tmo;

    if (!st)
        return -EINVAL;

    if (test_bit(STCP_FLAG_HS_EXIT_MODE_BIT, &st->flags))
        return -ESHUTDOWN;

    if (test_bit(STCP_FLAG_HS_COMPLETE_BIT, &st->flags))
        return 0;

    if (test_bit(STCP_FLAG_HS_FAILED_BIT, &st->flags))
        return -EPROTO;

    /* HS was never started -> connect/accept bug or early send */
    if (!test_bit(STCP_FLAG_HS_STARTED_BIT, &st->flags))
        return -EAGAIN;

    if (nonblock)
        return -EAGAIN;

    tmo = msecs_to_jiffies(timeout_ms);
    tmo = wait_for_completion_interruptible_timeout(&st->hs_done, tmo);
    if (tmo == 0)
        return -ETIMEDOUT;
    if (tmo < 0)
        return (int)tmo;

    if (test_bit(STCP_FLAG_HS_FAILED_BIT, &st->flags))
        return -EPROTO;

    if (st->hs_result < 0)
        return -EPROTO;

    if (!test_bit(STCP_FLAG_HS_COMPLETE_BIT, &st->flags))
        return -EAGAIN;

    return 0;
}

/* ----------------------------------------- */
/*  SENDMSG (STCP encrypted send)            */
/* ----------------------------------------- */
int stcp_rust_glue_proto_op_sendmsg(struct sock *sk, struct msghdr *msg, size_t len)
{
    ssize_t ret;
    struct stcp_sock *st;
    bool nonblock;
    int hsret;

    if (!sk || !msg)
        return -EINVAL;

    st = stcp_struct_get_st_from_sk(sk);
    nonblock = stcp_is_nonblock_send(sk, msg);

    if (!st) {
        SDBG("SendMSG: no stcp_sock");
        return -ENOTCONN;
    }

    if (test_bit(STCP_FLAG_HS_EXIT_MODE_BIT, &st->flags))
        return -ESHUTDOWN;

    /* Do NOT start HS here. Only wait / EAGAIN. */
    if (sk->sk_state != TCP_ESTABLISHED)
        return -EAGAIN;

    /* HS must be complete before encrypting */
    hsret = stcp_wait_hs_or_eagain(st, nonblock, 10 * 1000);
    if (hsret < 0)
        return hsret;

    if (!st->session || !st->sk) {
        SDBG("SendMSG: HS complete but session/sk missing session=%px sk=%px",
             st->session, st->sk);
        return -ENOTCONN;
    }

    if (len == 0)
        return 0;

    /* copy payload */
    char *kbuf = kzalloc(len, GFP_KERNEL);
    if (!kbuf)
        return -ENOMEM;

    /* copy_from_iter consumes iterator */
    {
        ssize_t copied = copy_from_iter(kbuf, len, &msg->msg_iter);
        if (copied != (ssize_t)len) {
            kfree(kbuf);
            return -EFAULT;
        }
    }

    /* prevent recursion if Rust does internal TCP send */
    set_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags);
    ret = rust_exported_session_sendmsg(st->session, (void *)st->sk, kbuf, len);
    clear_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags);

    kfree(kbuf);
    return (int)ret;
}

/* ----------------------------------------- */
/*  RECVMSG (STCP encrypted recv)            */
/* ----------------------------------------- */
int stcp_rust_glue_proto_op_recvmsg(struct sock *sk,
                                    struct msghdr *msg,
                                    size_t len,
                                    int flags,
                                    int *recv_len)
{
    ssize_t ret;
    struct stcp_sock *st;
    bool nonblock;
    int hsret;

    if (!sk || !msg)
        return -EINVAL;

    st = stcp_struct_get_st_from_sk(sk);

    SDBG("HOOK recvmsg ENTER sk=%px proto=%u user_data=%px state=%d",
         sk, sk->sk_protocol, sk->sk_user_data, sk->sk_state);

    if (!st) {
        SDBG("RecvMSG: no stcp_sock attached");
        return -ENOTCONN;
    }

    if (test_bit(STCP_FLAG_HS_EXIT_MODE_BIT, &st->flags))
        return -ESHUTDOWN;

    if (sk->sk_state != TCP_ESTABLISHED)
        return -EAGAIN;

    nonblock = !!(flags & MSG_DONTWAIT);

    /* Do NOT start HS here. Only wait / EAGAIN. */
    hsret = stcp_wait_hs_or_eagain(st, nonblock, 10 * 1000);
    if (hsret < 0)
        return hsret;

    if (!st->session || !st->sk) {
        SDBG("RecvMSG: HS complete but session/sk missing session=%px sk=%px",
             st->session, st->sk);
        return -ENOTCONN;
    }

    if (len == 0) {
        if (recv_len)
            *recv_len = 0;
        return 0;
    }

    char *kbuf = kzalloc(len, GFP_KERNEL);
    if (!kbuf)
        return -ENOMEM;

    SDBG("RecvMSG: receiving via rust session=%px tx_sk=%px kbuf=%px len=%zu",
         st->session, st->sk, kbuf, len);

    /* prevent recursion if Rust does internal TCP recv */
    set_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags);
    ret = rust_exported_session_recvmsg(st->session, (void *)st->sk,
                                        kbuf, len, /* blocking */ 0);
    clear_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags);

    if (ret > 0) {
        int cpret = memcpy_to_msg(msg, kbuf, ret);
        if (cpret < 0) {
            kfree(kbuf);
            return cpret;
        }
        if (recv_len)
            *recv_len = cpret;
        kfree(kbuf);
        return cpret;
    }

    kfree(kbuf);
    return (int)ret;
}

/* ----------------------------------------- */
/*  Destroy (STCP destroy)                   */
/* ----------------------------------------- */
void stcp_rust_cleanup_detached(struct stcp_sock *st)
{
    destroy_the_work_queue(st);

    if (st->session) {
        rust_exported_session_destroy(st->session);
        st->session = NULL;
    }

    stcp_struct_free_st(st);
}

struct stcp_sock *stcp_rust_try_detach_for_cleanup(struct sock *sk)
{
    struct stcp_sock *st = stcp_struct_get_st_from_sk_for_destroy(sk);
    if (!st)
        return NULL;

    if (cmpxchg(&st->magic, STCP_MAGIC_ALIVE, STCP_MAGIC_DEAD) != STCP_MAGIC_ALIVE)
        return NULL;

    stcp_detach_from_st(st);
    return st;
}

void stcp_rust_glue_proto_op_destroy(struct sock *sk)
{
    struct stcp_sock *st;

    st = stcp_rust_try_detach_for_cleanup(sk);

    if (WARN_ON_ONCE(in_interrupt())) {
        SDBG("Destroy in interrupt context; queueing destroy request");
        if (st)
            stcp_struct_session_destroy_request(st, REASON_DESTROY_FROM_INTERRUPT);
        return;
    }

    if (st) {
        SDBG("@ Destroy of sock: %px", sk);
        stcp_rust_cleanup_detached(st);
    }
}