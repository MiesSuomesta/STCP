// stcp_tcp_recv.c
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
#include <stcp/stcp_socket.h>
#include <stcp/stcp_tcp_low_level_operations.h>
#include <stcp/stcp_rx_transmission.h>

#include "testing/include/status_monitor.h"


#define STCP_RECV_POLL_TIMEOUT_MS 250

#define TCP_DEBUG 0

static int stcp_wait_for_data(int fd, int timeout_ms)
{
    struct zsock_pollfd pfd = {
        .fd = fd,
        .events = ZSOCK_POLLIN,
    };

    int rc = zsock_poll(&pfd, 1, timeout_ms);

    if (rc < 0) {
        LERR("poll error rc=%d errno=%d", rc, errno);
        return -errno;
    }

    if (rc == 0) {
        /* timeout = normaali */
        return -EAGAIN;
    }

    if (pfd.revents & ZSOCK_POLLIN) {
        return 1;
    }

    if (pfd.revents & ZSOCK_POLLERR) {
        return -ECONNRESET;
    }

    if (pfd.revents & ZSOCK_POLLHUP) {
        return -ECONNRESET;
    }

    if (pfd.revents & ZSOCK_POLLNVAL) {
        return -EBADF;
    }

    return -EAGAIN;
}

// Watchdog update, ei omassa headerissa, koska tämä on piilossa kaikilta.
void stcp_watchdog_update_activity(void);

static ssize_t _the_stcp_tcp_recv(
    int fd,
    int8_t *buf,
    size_t len,
    int non_blocking,
    int32_t flags,
    int *recv_len)
{
    // LDBG("RECV: Flags: %x", flags);
    int exact_mode = flags & STCP_RECV_FLAG_EXACT_MODE;
    int bloking = flags & STCP_RECV_FLAG_NON_BLOKING;

    //LDBG("RECV: Flags: exact: %d, non_bloking: %d",
    //    exact_mode, bloking);

    int my_flags = 
        STCP_RECV_FLAG_EXACT_MODE  |
        STCP_RECV_FLAG_NON_BLOKING;

    int real_fgs = flags & ~my_flags;

    if (fd < 0) {
        // LDBG("Fail: bad FD");
        return -EBADFD;
    }

    if (!buf) {
        // LDBG("stcp_tcp_recv: buf == NULL");
        return -ENOBUFS;
    }

    if (!recv_len) {
        // LDBG("stcp_tcp_recv: recv_len == NULL");
        return -EMSGSIZE;
    }

    if (!len) {
        // LDBG("stcp_tcp_recv: len == 0");
        return -EMSGSIZE;
    }

    int perr = stcp_get_pending_fd_error(fd);
    if (perr != 0) {
        LERR("Used FD has error pending! (%d)", perr);
        return -EAGAIN;
    }

    *recv_len = 0;

/*    SDBG("Sock[%d] Receiving message (%d bytes max, nb: %d)...",
        fd,
        (int)len,
        non_blocking);
*/
    if (non_blocking || bloking) {
  //      SDBG("Sock[%d] Set non bloking mode on at read flags", fd);
        real_fgs |= ZSOCK_MSG_DONTWAIT;
    }

    /*
     * Odotetaan että socketilla on dataa.
     * MQTT tarvitsee STREAM-semanttiikan:
     * palautetaan heti kun dataa tulee,
     * EI odoteta koko bufferin täyttymistä.
     */
    // LDBG("RECV: @use real flags: %x", real_fgs);

    ssize_t total = 0;
    ssize_t rc = 0;
    
do_re_read_from_sock:
    int pollret = stcp_wait_for_data(fd, STCP_RECV_POLL_TIMEOUT_MS);

    if (pollret == -EAGAIN) {
        return -EAGAIN;
    }

    if (pollret < 0) {
        // LDBG("STCP: poll ret %d", pollret);
        return pollret;
    }

    size_t bytes_to_read = len - total;

/*
    LERR(
        "STCP: RECV fd=%d len=%d total=%d left=%d flags=0x%x real=0x%x exact=%d",
        fd,
        (int)len,
        (int)total,
        (int)(bytes_to_read),
        flags,
        real_fgs,
        exact_mode
    );
*/
    if (bytes_to_read > 0) {
        errno = 0;
        rc = zsock_recv(
            fd,
            buf + total,
            bytes_to_read,
            real_fgs
        );
    } else {
        rc = 0;
    }
    
/*    LDBG("STCP RECV CALLED, ret=%d total=%d errno=%d",
        (int)rc, 
        (int)total,
        errno);
*/
    /*
     * Socket closed cleanly.
     */
    if (rc == 0) {

        if (bytes_to_read == 0) {
            // LDBG("STCP RECV: got all!");
        } else {
#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
            stcp_statistics_inc(STAT_RECV_ZERO, 1);
#endif

            //LWRN("Socket disconnected");
            //LERR("RECV EXIT rc=%d total=%d", rc, total);
            return -ENOTCONN;
        }
    }

    /*
     * Error path.
     */
    if (rc < 0) {

        /*
         * Nonblocking socket:
         * ei dataa juuri nyt.
         */
        
        if (errno == EAGAIN ||
            errno == EWOULDBLOCK) {
            // LDBG("Recv returned retry flag....");
            LERR("RECV EXIT rc=%d total=%d", -errno, total);
            return -errno; // Palautetaan AS IS
        }

        LERR("Recv failed rc=%d errno=%d", rc, errno);

#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
        stcp_statistics_inc(STAT_RECV_ERRORS, 1);
#endif

        dump_socket_error(fd);

        LERR("RECV EXIT rc=%d total=%d", -errno, total);
        return -errno;
    }

    // LDBG("STCP: RECV in exact mode: %d", exact_mode);
    // Tässä laittaa ylös kuinka paljon luettiin tällä kerralla.
    total += rc;

    if (exact_mode) {
        if (total < len) {
            // LDBG("STCP: RECV reread.. %d < %d", total, len);
            k_sleep(K_MSEC(5));
            goto do_re_read_from_sock;
        }
    }

    /*
     * Success.
     */
    if (recv_len != NULL) {
//        LDBGBIG("STCP: RECV updating recv_len pointer with %d", (int)total);
        *recv_len = total;
    }

#if CONFIG_STCP_STATUS_MONITOR_STATISTICS
    stcp_statistics_inc(STAT_RECV_CALLS, 1);
    stcp_statistics_inc(STAT_RX_BYTES, total);
#endif

#if CONFIG_STCP_WATCHDOG_ENABLE
    stcp_watchdog_update_activity();
#endif

#if CONFIG_STCP_STATISTICS
    /*
     * Statistics monitor hook.
     */
    int stats =
        stcp_statistic_monitor_check_for_command_reqest(
            buf,
            total
        );

    if (stats > 0) {

        // LDBG("Got request for statistics");

        stcp_statistic_monitor_send_state_to(fd);

        LERR("RECV EXIT rc=%d total=%d", rc, total);
        return -EAGAIN;
    }
#endif

#if TCP_DEBUG
    // LDBG(".----<[MESSAGE]>-------------------------------------------->");
    // LDBG("|  ✅ Received data %d bytes", (int)total);

/*    stcp_hexdump_ascii(
        "RX Raw dump",
        buf,
        total
    );
*/
    // LDBG("'----------------------------------------------------------'");
#endif

//    LERR("RECV EXIT rc=%d total=%d", rc, total);
    return total;
}

intptr_t stcp_tcp_recv(void *sock_vp,
                        uint8_t *buf,
                        uintptr_t len,
                        int32_t non_blocking,
                        int32_t flags,
                        int *recv_len)
{
    struct kernel_socket *sock = sock_vp;

    if (!sock) {
        LERR("stcp_tcp_recv: sock == NULL");
        return -ENODEV;
    }

    struct stcp_ctx *ctx = sock->kctx;
    
    if (stcp_is_context_valid(ctx) < 1) {
        LERR("stcp_tcp_recv: ctx not valid");
        return -ENOMEDIUM;
    }

    if (!buf) {
        LERR("stcp_tcp_recv: buf == NULL, INVAL C");
        return -EINVAL;
    }

    if (sock->fd < 0) {
        LERR("stcp_tcp_recv: No FD!, INVAL D");
        return -EINVAL;
    }

    if (atomic_get(&ctx->closing)) {
        LWRN("Socket closing ...");
        return -EINPROGRESS;
    };

    *recv_len = 0;
    memset(buf, 0, len); // tärkeä!
    ssize_t rc = _the_stcp_tcp_recv(
        sock->fd,
        (int8_t *)buf,
        (size_t)len,
        non_blocking,
        flags,          // flags    
        recv_len
    );

    if ((int)rc > 0) {
        // LDBG("STCP RECV Dump:");
        // stcp_hexdump_ascii("RUST RX Buf", buf, rc);
    }

    if (rc < 0) {
        LWRN("STCP RECV Got error %d, errno=%d", rc, errno);
        if (rc == -EAGAIN || rc == -EWOULDBLOCK) {
            LWRN("Errno: %d => AGAIN", rc);
            return -EAGAIN;
        }

        if (rc == -ENOTCONN) {
            LWRN("STCP RECV: Got no connection?");
            return -ENOTCONN;
        }

        LWRN("Used FD has error pending! (%d)", (int)rc);
    }

    return (intptr_t)rc;
}

intptr_t stcp_tcp_recv_via_fd(int fd,
                       uint8_t *buf,
                       uintptr_t len,
                       int32_t non_blocking,
                       uint32_t flags,
                       int *recv_len)
{
    if (!buf) {
        LERR("stcp_tcp_recv_via_fd: buf == NULL, INVAL A");
        return -EINVAL;
    }

    if (fd < 0) {
        LERR("stcp_tcp_recv_via_fd: No FD!, INVAL B");
        return -EINVAL;
    }

    *recv_len = 0;
    memset(buf, 0, len); // tärkeä!
    ssize_t rc = _the_stcp_tcp_recv(
        fd,
        (int8_t *)buf,
        (size_t)len,
        non_blocking,
        flags,          // flags    
        recv_len
    );

    if ((int)rc > 0) {
//        stcp_hexdump_ascii("RUST RX Buf", buf, rc);
    }

    if (rc < 0) {
        
        if (rc == -EAGAIN || rc == -EWOULDBLOCK) {
            LWRN("STCP RECV error mapping: %d => EAGAIN", rc);
            return -EAGAIN;
        }

        LWRN("STCP RECV: FD %d has error pending! (%d)", fd, (int)rc);
        return (intptr_t)rc;
    }

    return (intptr_t)rc;
}
