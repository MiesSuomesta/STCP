// stcp_tcp_send.c
#include <zephyr/logging/log.h>

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>
#include <stdint.h>
#include <errno.h>

#include <stcp/debug.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_tcp_low_level_operations.h>

LOG_MODULE_REGISTER(stcp_low_level_send_operations, LOG_LEVEL_INF);

#define TCP_DEBUG 1

ssize_t stcp_tcp_send_iovec(struct kernel_socket *sock,
                            void *msgVP,
                            int flags)
{
    struct msghdr *msg = msgVP;

    if (!sock) {
        LDBG("Socket not set");
        return -EBADMSG;
    }

    if (!msg) {
        LDBG("message not set");
        return -EBADMSG;
    }

    int fd = sock->fd;

    if (fd < 0) {
        LDBG("Fail: bad FD");
        return -EBADFD;
    }

    if (!msg || !msg->msg_iov || msg->msg_iovlen == 0) {
        LDBG("Invalid msg struct");
        return -EINVAL;
    }

    int64_t start   = k_uptime_get();
    int64_t timeout = 10 * 1000; // 10 sec

    ssize_t total = 0;
    errno = 0;

    /* Lasketaan kokonaispituus */
    size_t remaining = 0;
    for (int i = 0; i < msg->msg_iovlen; i++) {
        remaining += msg->msg_iov[i].iov_len;
    }

    while (remaining > 0) {

        ssize_t sent = zsock_sendmsg(fd, msg, flags);

        if (sent < 0) {

            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if (k_uptime_get() - start > timeout) {
                    LERR("send timeout (%d ms)", (int)timeout);
                    return -ETIMEDOUT;
                }

                k_sleep(K_MSEC(10));
                continue;
            }

            if (errno == ENOMEM || errno == ENOBUFS) {
                k_sleep(K_MSEC(50));
                continue;
            }

            if (errno == ECONNRESET || errno == ENOTCONN || errno == EPIPE) {
                LERR("Socket dead: errno=%d", errno);
                return -errno;
            }

            return -errno;
        }

        total     += sent;
        remaining -= sent;

        /* Päivitetään iovec offset */
        size_t consumed = sent;

        for (int i = 0; i < msg->msg_iovlen && consumed > 0; i++) {

            if (consumed >= msg->msg_iov[i].iov_len) {
                consumed -= msg->msg_iov[i].iov_len;
                msg->msg_iov[i].iov_base =
                    (uint8_t *)msg->msg_iov[i].iov_base +
                    msg->msg_iov[i].iov_len;
                msg->msg_iov[i].iov_len = 0;
            } else {
                msg->msg_iov[i].iov_base =
                    (uint8_t *)msg->msg_iov[i].iov_base + consumed;
                msg->msg_iov[i].iov_len -= consumed;
                consumed = 0;
            }
        }
    }

#if TCP_DEBUG
    LDBG("✅ sendmsg fd=%d total=%d", fd, (int)total);
#endif

    return total;
}

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

    if (!buf) {
        LDBG("No buff %p", buf);
        return -EINVAL;
    }
    /* zephyrin sendmsg, puskee suoraan TCP:lle */

    int64_t start = k_uptime_get();
    int64_t timeout = 10*1000; // 10 seconds

    errno = 0;
    stcp_hexdump_ascii("RUST TX Buf", buf, len);
    ssize_t total = 0;

    while (total < len) {
        ssize_t sent = zsock_send(fd, buf + total, len - total, flags);

        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if (k_uptime_get() - start > timeout) { // timeout
                    LERR("send timeout (%d seconds)", (int)timeout);
                    return -ETIMEDOUT;
                }
                // Ei busy looppia, annetaan masiinan hengähtää välillä..
                k_sleep(K_MSEC(10));
                continue;  // non-blocking case
            }

            // Jos laite on kiireinen, ootellaan hetki..
            if (errno == ENOMEM || errno == ENOBUFS) {
                k_sleep(K_MSEC(50));
                continue;
            }

            if (errno == ECONNRESET || errno == ENOTCONN || errno == EPIPE) {
                LERR("Socket dead: errno=%d", errno);
                return -errno;
            }
            
            return -errno;
        }

        total += sent;
    }

//	k_mutex_unlock(&ctx->lock);

#if TCP_DEBUG
    LDBG(".----<[MESSAGE]>------------------------------------------------------------>\n");
    LDBG("|  ✅ Sent data to fd: %d, %d bytes (errno: %d)", fd, (int)total, errno);
    stcp_hexdump_ascii("TX Raw dump", buf, total);
    LDBG("'----------------------------------------------------------------------'\n");
#endif

    return total;    // >=0: tavujen määrä, <0: -errno
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

    if (sock->fd < 0) {
        LERR("stcp_tcp_send: No FD!");
        return -EINVAL;
    }

    int perr = stcp_get_pending_fd_error(sock->fd);
    if (perr != 0) {
        if (errno == 128) {
            LDBG("Git 128 => returning -EAGAIN");
            return -EAGAIN;
        }
        LERR("Used FD has error pending! (%d)", perr);
        return -EINVAL;
    }

    //if (atomic_get(&ctx->closing)) {
    //    LERR("Socket closing ...");
    //    return -EAGAIN;
    //};

    int rc = _the_stcp_tcp_send(sock->fd, buf, len, 0);
    if (rc < 0) {
        if (errno == 128) {
            LDBG("Git 128 => returning -EAGAIN");
            return -EAGAIN;
        }
        LERR("Used FD has error pending! (%d)", perr);
        return -EINVAL;
    }

    return (intptr_t)rc;
}

