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
#include "testing/include/status_monitor.h"



#define TCP_DEBUG 0

intptr_t stcp_tcp_send_iovec(void *sock_vp, void *msg_vp, int flags)
{
    struct kernel_socket *sock = sock_vp;
    struct msghdr *msg = msg_vp;

    if (!sock) {
        // LDBG("Socket not set");
        return -EBADMSG;
    }

    if (!msg) {
        // LDBG("message not set");
        return -EBADMSG;
    }

    int fd = sock->fd;

    if (fd < 0) {
        // LDBG("Fail: bad FD");
        return -EBADFD;
    }

    if (!msg || !msg->msg_iov || msg->msg_iovlen == 0) {
        // LDBG("Invalid msg struct");
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

            stcp_statistics_inc(STAT_ERRORS, 1);
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

    stcp_statistics_inc(STAT_TX_BYTES, total);

    return total;
}

static ssize_t _the_stcp_tcp_send(int fd, const int8_t *buf, size_t len, int flags)
{
    int exit_code = 0;
    if (fd < 0) {
        // LDBG("Fail: bad FD");
        return -EBADFD;
    }

    if (!buf) {
        // LDBG("No buff %p", buf);
        return -EINVAL;
    }
    /* zephyrin sendmsg, puskee suoraan TCP:lle */

    int64_t start = k_uptime_get();
    int64_t timeout = 10*1000; // 10 seconds

    errno = 0;
    stcp_hexdump_ascii("RUST TX Buf", buf, len);
    ssize_t total = 0;

#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
    stcp_statistics_inc(STAT_SEND_CALLS, 1);
#endif

    while (total < len) {
        //LDBGBIG("STCP: ZSOCK_SEND via %d", fd);
        ssize_t sent = zsock_send(fd, buf + total, len - total, flags);
        //LDBGBIG("STCP: ZSOCK_SEND via %d ret: %d errno=%d", fd, sent, errno);

        if (sent < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if (k_uptime_get() - start > timeout) { // timeout
                    LERR("send timeout (%d ms)", (int)timeout);
                    exit_code = -ETIMEDOUT;
                    goto do_exit;
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

#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
            stcp_statistics_inc(STAT_ERRORS, 1);
#endif
            if (errno == ECONNRESET || errno == ENOTCONN || errno == EPIPE) {
                LERR("Socket dead: errno=%d", errno);
                exit_code = -errno;
                goto do_exit;
            }
            
            return -errno;
        }

        total += sent;
    }

//	k_mutex_unlock(&ctx->lock);

#if TCP_DEBUG
    // LDBG(".----<[MESSAGE]>------------------------------------------------------------>\n");
    // LDBG("|  ✅ Sent data to fd: %d, %d bytes (errno: %d)", fd, (int)total, errno);
    // stcp_hexdump_ascii("TX Raw dump", buf, total);
    // LDBG("'----------------------------------------------------------------------'\n");
#endif

#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
    stcp_statistics_inc(STAT_TX_BYTES, total);
#endif
    return total;    // >=0: tavujen määrä, <0: -errno

do_exit:

#if TCP_DEBUG
    // LDBG(".----<[MESSAGE FAILURE]>------------------------------------------------------------>\n");
    LDBG("|  ✅ Sent data to fd: %d, %d bytes (errno: %d)", fd, (int)total, errno);
    // stcp_hexdump_ascii("TX Raw dump", buf, total);
    // LDBG("'----------------------------------------------------------------------'\n");
#endif

#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
    stcp_statistics_inc(STAT_SEND_ERRORS, 1);
#endif

    return exit_code; 

}

// Watchdog update, ei omassa headerissa, koska tämä on piilossa kaikilta.
void stcp_watchdog_update_activity(void);

intptr_t stcp_tcp_send_via_fd(int fd, const uint8_t *buf, uintptr_t len)
{

    if (!buf) {
        LERR("stcp_tcp_send: buf == NULL");
        return -EINVAL;
    }

    if (len < 1) {
        LERR("stcp_tcp_send: len < 1");
        return -EINVAL;
    }
    
    //stcp_hexdump_ascii("STCP TX", buf, len);
    //stcp_dump_bt();

    if (fd < 0) {
        LERR("stcp_tcp_send_via_fd: No FD!");
        return -EINVAL;
    }

    int perr = stcp_get_pending_fd_error(fd);
    if (perr != 0) {
        LERR("Used FD has error pending! (%d)", perr);
        return perr;
    }

    //if (atomic_get(&ctx->closing)) {
    //    LERR("Socket closing ...");
    //    return -EAGAIN;
    //};

    int rc = _the_stcp_tcp_send(fd, buf, len, 0);
    stcp_watchdog_update_activity();
    if (rc < 0) {
        LERR("Tcp sending error %d, errno=%d", rc, errno);
        return rc;
    }
    
    return (intptr_t)rc;
}



intptr_t stcp_tcp_send(void *sock_vp, const uint8_t *buf, uintptr_t len)
{

//    LDBGBIG("STCP: STCP SEND CALLED....");

    if (!buf) {
        LERR("stcp_tcp_send: buf == NULL");
        return -ENOBUFS;
    }

    if (len < 1) {
        LERR("stcp_tcp_send: len < 1");
        return -EMSGSIZE;
    }
    
    stcp_hexdump_ascii("STCP TX", buf, len);
    stcp_dump_bt();

    struct kernel_socket *sock = sock_vp;
    if (!sock) {
        LERR("stcp_tcp_send: sock == NULL");
        return -ENODEV;
    }

    struct stcp_ctx *ctx = sock->kctx;
    
    if (!ctx) {
        LERR("stcp_tcp_send: ctx == NULL");
        return -ENOMEDIUM;
    }

    if (sock->fd < 0) {
        LERR("stcp_tcp_send: No FD!");
        return -EBADFD;
    }

    int perr = stcp_get_pending_fd_error(sock->fd);
    if (perr < 0) {
        LERR("Used FD has error pending! (%d)", perr);
        return perr;
    }

    //if (atomic_get(&ctx->closing)) {
    //    LERR("Socket closing ...");
    //    return -EAGAIN;
    //};

    //LDBGBIG("STCP: SENDING via fd: %d", sock->fd);
    int rc = _the_stcp_tcp_send(sock->fd, buf, len, 0);
    //LDBGBIG("STCP: SENT via fd: %d ret: %d", sock->fd, rc);

    stcp_watchdog_update_activity();
    if (rc < 0) {
        LERR("STCP Send error: %d", rc);
    }
    return (intptr_t)rc;
}

