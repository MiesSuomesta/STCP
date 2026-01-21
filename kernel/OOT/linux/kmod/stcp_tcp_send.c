// stcp_tcp_send.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <linux/net.h>
#include <linux/errno.h>

#include <stcp/debug.h>
#include <net/sock.h>
#include <net/inet_common.h>
#include <net/inet_connection_sock.h>
#include <net/protocol.h>
#include <net/tcp.h>
#include <linux/inet.h>

#include <stcp/stcp_socket_struct.h>
#include <stcp/stcp_tcp_low_level_operations.h>
#include <stcp/stcp_socket_struct.h>
#include <stcp/proto_layer.h>
#include <stcp/stcp_proto_ops_helpers.h>

// RUSTISTA KUTSUTAAN .. EI C Puolelta!
#define TCP_DEBUG 1

ssize_t stcp_tcp_send(struct sock *sk, u8 *buf, size_t len)
{
   
    SDBG("SEND sk=%px state=%d", sk, sk && sk->sk_state);

    struct stcp_sock *st = stcp_struct_get_st_from_sk(sk);
    if (!st || !st->session)
        return -EINVAL;

    DEBUG_INCOMING_STCP_STATUS(st);

    if (!test_bit(STCP_FLAG_HS_COMPLETE_BIT, &st->flags) &&
        !test_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags)) {
        SDBG("Not doing anything...");
        return -EAGAIN; /* tai -EPROTO */
    }

    struct kvec iov;
    struct msghdr msg = {0};
    ssize_t sent;

    SDBG("TCP/IO/SEND: st=%px session=%px stcp_sk=%px",
        st, st ? st->session : NULL, sk);

    if (!sk || !buf)
        return -EINVAL;

    iov.iov_base = buf;
    iov.iov_len  = len;

    iov_iter_kvec(&msg.msg_iter, WRITE, &iov, 1, len);

    /* TCP, stream socket, ei erikoisflageja */
    msg.msg_flags = MSG_NOSIGNAL;

    /* kernel_sendmsg, puskee suoraan TCP:lle */
    sent = orginal_tcp_sendmsg(sk, &msg, len);

#if TCP_DEBUG
    pr_emerg_ratelimited(".----<[MESSAGE]>------------------------------------------------------------>\n");
    pr_emerg_ratelimited("|  ✅ Sent data: %d bytes ", (int)sent);
    pr_emerg_ratelimited("'----------------------------------------------------------------------'\n");
#endif

    return sent;    // >=0: tavujen määrä, <0: -errno
}
