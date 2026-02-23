// stcp_tcp_recv.c
#include <zephyr/logging/log.h>

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>
#include <stdint.h>
#include <errno.h>

#include "debug.h"
#include "stcp.h"
#include "stcp_struct.h"
#include "stcp_bridge.h"

LOG_MODULE_REGISTER(stcp_low_level_recv_operations, LOG_LEVEL_INF);

#define STCP_RECV_POLL_TIMEOUT_MS (10*1000)

#define TCP_DEBUG 1

static ssize_t _the_stcp_tcp_recv(
    int fd,
    int8_t *buf,
    size_t len,
    int non_blocking,
    int32_t flags,
    int *recv_len)
{
    if (fd < 0) {
        LDBG("Fail: bad FD");
        return -EBADFD;
    }

    if (!buf) {
        LDBG("stcp_tcp_recv: buf == NULL\n");
        return -EINVAL;
    }

    if (!recv_len) {
        LDBG("stcp_tcp_recv: recv_len == NULL\n");
        return -EINVAL;
    }

    int perr = stcp_get_pending_fd_error(fd);
    if (perr != 0) {
        LERR("Used FD has error pending! (%d)", perr);
        return -ENOTCONN;
    }

    *recv_len = 0;

    SDBG("Sock[%d] Receiving message (%d bytes max, nb: %d)...", fd, (int)len, non_blocking);

    if (non_blocking == 0) {
        //blocking 
        flags |= ZSOCK_MSG_WAITALL;
        LDBG("Setting blocking ON");
    } else {
        flags |= ZSOCK_MSG_DONTWAIT;
        LDBG("Setting blocking OFF");
    }

    int ret = 0;
    size_t total = 0;
    while (total < len) {
        errno = 0;
        ssize_t r = zsock_recv(fd, buf + total, len - total, flags);

        if (r > 0) {
            total += r;
            continue;
        }

        if (r == 0) {

#if TCP_DEBUG
            LDBG(".----<[MESSAGE]>------------------------------------------------------------>\n");
            LDBG("| Peer closed, errno: %d ", errno);
            LDBG("'----------------------------------------------------------------------'\n");
#endif

            return 0; // peer closed
        }

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            k_sleep(K_MSEC(5));
            continue;
        }

#if TCP_DEBUG
            LDBG(".----<[MESSAGE]>------------------------------------------------------------>\n");
            LDBG("| Error while reading data, errno: %d ", errno);
            LDBG("'----------------------------------------------------------------------'\n");
#endif

        return -errno;
    }

#if TCP_DEBUG
        LDBG(".----<[MESSAGE]>------------------------------------------------------------>\n");
        LDBG("|  ✅ Received data %d bytes, errno: %d ", (int)recv_len, errno);
        stcp_hexdump_ascii("RX Raw dump", buf, total);
        LDBG("'----------------------------------------------------------------------'\n");
#endif

    *recv_len = total;
    return total;
}

intptr_t stcp_tcp_recv(struct kernel_socket *sock,
                       uint8_t *buf,
                       uintptr_t len,
                       int32_t non_blocking,
                       uint32_t flags,
                       int *recv_len)
{
    
    if (!sock) {
        LERR("stcp_tcp_send: sock == NULL");
        return -EINVAL;
    }

    struct stcp_ctx *ctx = sock->kctx;
    
    if (stcp_is_context_valid(ctx) < 1) {
        LERR("stcp_tcp_send: ctx not valid");
        return -EINVAL;
    }

    if (!buf) {
        LERR("stcp_tcp_send: buf == NULL");
        return -EINVAL;
    }

    if (sock->fd < 0) {
        LERR("stcp_tcp_send: No FD!");
        return -EINVAL;
    }

    /*
    if (atomic_get(&ctx->closing)) {
        LERR("Socket closing ...");
        return -EINPROGRESS;
    };
    */
    *recv_len = 0;

    ssize_t rc = _the_stcp_tcp_recv(
        sock->fd,
        (int8_t *)buf,
        (size_t)len,
        non_blocking,
        flags,          // flags    
        recv_len
    );

    stcp_hexdump_ascii("RUST RX Buf", buf, recv_len);

    if (rc < 0) {
        return (intptr_t)rc;
    }

    return (intptr_t)rc;   // <- TÄRKEÄ
}
