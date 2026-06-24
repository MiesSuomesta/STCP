// stcp_tcp_recv.c
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
#include <stcp/stcp_tcp_low_level_operations.h>
#include <stcp/proto_layer.h>
#include <stcp/stcp_socket_struct.h>
#include <stcp/stcp_proto_ops_helpers.h>

#define TCP_DEBUG 1

// rv > 0, bytet
// rv < 0, errori
ssize_t stcp_tcp_recv(
    struct sock *sk,
    u8 *buf,
    size_t len,
    int non_blocking,
    u32 flags,
    int *recv_len)
{
    SDBG("RECV sk=%px state=%d", sk, sk && sk->sk_state);

    struct stcp_sock *st = stcp_struct_get_st_from_sk(sk);
    if (!st || !st->session)
        return -EINVAL;

    DEBUG_INCOMING_STCP_STATUS(st);

    if (!sk) {
        pr_err("stcp_tcp_recv: sk == NULL\n");
        return -EINVAL;
    }
 

    SDBG("IO/RECV: sk=%px protocol=%d prot=%px", sk, (int)sk->sk_protocol, sk->sk_prot);

    if (!test_bit(STCP_FLAG_HS_COMPLETE_BIT, &st->flags) &&
        !test_bit(STCP_FLAG_INTERNAL_IO_BIT, &st->flags)) {
        SDBG("Not doing anything...");
        return -EAGAIN; /* tai -EPROTO */
    }


    if (!buf) {
        pr_err("stcp_tcp_recv: buf == NULL\n");
        return -EINVAL;
    }

    if (!recv_len) {
        pr_err("stcp_tcp_recv: recv_len == NULL\n");
        return -EINVAL;
    }

    // Rakennetaan viestin vastaanotto vektori + msg...

    struct msghdr msg;
    struct kvec iov;

    iov.iov_base = (void *)buf;
    iov.iov_len  = len;

    if (!sk->sk_wq ) {
        SDBG("Socket can not sleep, cause it has no queue => non blocking enforced.");
        non_blocking = 1;
    }

    if (non_blocking) {
        flags |= MSG_DONTWAIT;
    } else {
        flags |= MSG_WAITALL;
    }

    if (flags & MSG_DONTWAIT) {
        SDBG("Sock[%px] Receiving message (DONTWAIT)", sk);
    }

    if (flags & MSG_WAITALL) {
        SDBG("Sock[%px] Receiving message (WAITALL)", sk);
    }

    memset(&msg, 0, sizeof(msg));
    msg.msg_name       = NULL;
    msg.msg_namelen    = 0;
    msg.msg_control    = NULL;
    msg.msg_controllen = 0;
    msg.msg_flags      = flags;

    iov_iter_kvec(&msg.msg_iter, READ, &iov, 1, len);

    *recv_len = 0;
    SDBG("Sock[%px] Receiving message (%d bytes max, NB: %d)...", sk, (int)len, non_blocking);
    int ret = orginal_tcp_recvmsg(sk, &msg, len, flags, recv_len);
    SDBG("Sock[%px] Message fetching ret: %d ...", sk, (int)ret);

    if (ret > 0) {
        SDBG("Sock[%px] Message copying to userland: %d bytes...", sk, (int)ret);
        *recv_len = (int)ret;

#if TCP_DEBUG
        pr_emerg_ratelimited(".----<[MESSAGE]>------------------------------------------------------------>\n");
        pr_emerg_ratelimited("|  ✅ Received data %d bytes ", (int)ret);
        pr_emerg_ratelimited("'----------------------------------------------------------------------'\n");
#endif

    } else {
        if (ret == 0) {
            SDBG("Sock[%px] Peer closed.", sk);
        } else {
            if (ret == -EWOULDBLOCK) {
                SDBG("Sock[%px] Would block (EWOULDBLOCK)", sk);
            } else if (ret == -EAGAIN) {
                SDBG("Sock[%px] Try again (EAGAIN).", sk);
            } else {
                SDBG("Sock[%px] Error: %d", sk, (int)ret);
            }
        }
    }
    
    SDBG("Sock[%px] Message fetching final ret: %d ...", sk, (int)ret);
    return ret;
}
