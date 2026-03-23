
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/net.h>
#include <linux/errno.h>

#include <net/sock.h>
#include <net/inet_common.h>
#include <net/inet_connection_sock.h>
#include <net/protocol.h>
#include <net/tcp.h>
#include <linux/inet.h>

#include <stcp/debug.h>
#include <stcp/proto_layer.h>   // Rust proto_ops API
#include <stcp/stcp_misc.h>

#include <stcp/stcp_socket_struct.h>
#include <stcp/rust_exported_functions.h>
#include <stcp/proto_operations.h>
#include <stcp/handshake_worker.h>
#include <stcp/stcp_tcp_low_level_operations.h>

inline bool stcp_glue_call_is_nonblock(const struct sock *sk, int flags)
{
    if (flags & MSG_DONTWAIT)
        return true;

    if (sk && sk->sk_socket && sk->sk_socket->file &&
        (sk->sk_socket->file->f_flags & O_NONBLOCK))
        return true;

    return false;
}

/* Palauttaa 0 jos HS ok, tai negatiivisen errno:n */
inline int stcp_glue_ensure_handshake(struct stcp_sock *st,
                                        struct sock *sk,
                                        int flags,
                                        int reason)
{
    bool nonblock;

    if (!st)
        return -ENOTCONN;

    nonblock = stcp_glue_call_is_nonblock(sk, flags);

    /* Varmista että handshake on jonossa (idempotent) */
    stcp_rust_queue_work_for_stcp_hanshake(st, 0, reason);

    if (test_bit(STCP_FLAG_HS_COMPLETE_BIT, &st->flags))
        return 0;

    if (nonblock)
        return -EAGAIN;

    if (!wait_for_completion_interruptible_timeout(&st->hs_done, 10 * HZ))
        return -ETIMEDOUT;

    if (st->hs_result < 0)
        return st->hs_result;

    if (test_bit(STCP_FLAG_HS_COMPLETE_BIT, &st->flags)) {
        SDBG("Ensure: Complete");
        return 0;
    } 
    
    SDBG("Ensure: -EAGAIN");
    return -EAGAIN;    
}
