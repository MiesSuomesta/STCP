// stcp_tcp_send.c
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
#include "stcp_net.h"
#include "stcp_bridge.h"
LOG_MODULE_REGISTER(stcp_low_level_send_operations, LOG_LEVEL_INF);

// RUSTISTA KUTSUTAAN .. EI C Puolelta!
#define TCP_DEBUG 1

static ssize_t _the_stcp_tcp_send(int fd, int8_t *buf, size_t len, int flags)
{
    if (fd < 0) {
        LDBG("Fail: bad FD");
        return -EBADFD;
    }
    
/*
    int perr = stcp_get_pending_fd_error(fd);
    if (perr != 0) {
        LERR("Used FD has error pending! (%d)", perr);
        return -ENOTCONN;
    } else {
        LDBG("Error check: FD %d has no error pending...", fd);
    }
*/
    //Lukko pidetään C puolella kiinni
    ssize_t sent;

    if (!fd || !buf) {
        LDBG("No buff or fd!, %p, %d", buf, fd);
        return -EINVAL;
    }
    /* zephyrin sendmsg, puskee suoraan TCP:lle */

    errno = 0;
    stcp_hexdump_ascii("RUST TX Buf", buf, len);
    sent = zsock_send(fd, buf, len, flags);

//	k_mutex_unlock(&ctx->lock);

#if TCP_DEBUG
    LDBG(".----<[MESSAGE]>------------------------------------------------------------>\n");
    LDBG("|  ✅ Sent data to fd: %d, %d bytes (errno: %d)", fd, (int)sent, errno);
    stcp_hexdump_ascii("TX Raw dump", buf, len);
    LDBG("'----------------------------------------------------------------------'\n");
#endif

    return sent;    // >=0: tavujen määrä, <0: -errno
}

intptr_t stcp_tcp_send(struct kernel_socket *sock, const uint8_t *buf, uintptr_t len)
{
    
    if (!sock) {
        LERR("stcp_tcp_send: sock == NULL");
        return -EINVAL;
    }

    struct stcp_ctx *ctx = sock->kctx;
    
    if (!ctx) {
        LERR("stcp_tcp_send: ctx == NULL");
        return -EINVAL;
    }

    if (!buf) {
        LERR("stcp_tcp_send: buf == NULL");
        return -EINVAL;
    }

    if (!sock->fd) {
        LERR("stcp_tcp_send: No FD!");
        return -EINVAL;
    }

    int perr = stcp_get_pending_fd_error(sock->fd);
    if (perr != 0) {
        LERR("Used FD has error pending! (%d)", perr);
        return -ENOTCONN;
    }

    //if (atomic_get(&ctx->closing)) {
    //    LERR("Socket closing ...");
    //    return -EAGAIN;
    //};

    int rc = _the_stcp_tcp_send(sock->fd, buf, len, 0);
    if (rc < 0) {
        return -errno;
    }

    return (intptr_t)rc;
}

