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
#include <stcp/stcp_tcp_low_level_operations.h>
#include <stcp/stcp_rx_transmission.h>
#include <status_monitor.h>


#define STCP_RECV_POLL_TIMEOUT_MS (10*1000)

#define TCP_DEBUG 1



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

    if (non_blocking != 0) {
        flags |= ZSOCK_MSG_DONTWAIT;
    }

    errno = 0;
    ssize_t r = zsock_recv(fd, buf, len, flags);
    stcp_statistics_inc(STAT_RECV_CALLS, 1);
    stcp_watchdog_update_activity();
    
    // Checkatan statistiikka pyyntö
#if CONFIG_STCP_STATISTICS
    if (r > 0) {
        int stats = stcp_statistic_monitor_check_for_command_reqest(buf, r);
        if (stats > 0) {
            LDBG("Got request for statistics....");
            stcp_statistic_monitor_send_state_to(fd);
            return -EAGAIN; // Notify with EAGAIN 
        }
    }
#endif
    if (r > 0) {
        *recv_len = r;
         stcp_statistics_inc(STAT_RX_BYTES, r);
   
#if TCP_DEBUG
        LDBG(".----<[MESSAGE]>------------------------------------------------------------>\n");
        LDBG("|  ✅ Received data %d bytes, errno: %d ", (int)r, errno);
        stcp_hexdump_ascii("RX Raw dump", buf, r);
        LDBG("'----------------------------------------------------------------------'\n");
#endif

        return r;
    } else {
        LDBG("Got error while recving: %d", r);
        stcp_statistics_inc(STAT_RECV_ERRORS, 1);
        dump_socket_error(fd);
    }

    if (r == 0) {
        stcp_statistics_inc(STAT_RECV_ZERO, 1);
        return -ENOTCONN;
    }

    if (errno == EAGAIN || errno == EWOULDBLOCK) {
        return -EAGAIN;
    }

    return -errno;
}

intptr_t stcp_tcp_recv(void *sock_vp,
                       uint8_t *buf,
                       uintptr_t len,
                       int32_t non_blocking,
                       uint32_t flags,
                       int *recv_len)
{
    struct kernel_socket *sock = sock_vp;
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
        stcp_hexdump_ascii("RUST RX Buf", buf, rc);
    }

    if (rc > 0) {

        printk("STCP TCP RX %d bytes\n", rc);

        for (int i = 0; i < rc; i++) {
            printk("%02X ", buf[i]);
        }

        printk("\n");
    }

    if (rc < 0) {
 
        if (errno == 128) {
            LWRN("Git 128 => returning -EAGAIN");
            return -EAGAIN;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            LWRN("Errno: %d => AGAIN", errno);
            return -EAGAIN;
        }

        LWRN("Used FD has error pending! (%d)", (int)rc);
        return (intptr_t)rc;
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
        LERR("stcp_tcp_recv_via_fd: buf == NULL");
        return -EINVAL;
    }

    if (fd < 0) {
        LERR("stcp_tcp_recv_via_fd: No FD!");
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
        stcp_hexdump_ascii("RUST RX Buf", buf, rc);
    }

    if (rc > 0) {

        printk("STCP TCP RX %d bytes\n", rc);

        for (int i = 0; i < rc; i++) {
            printk("%02X ", buf[i]);
        }

        printk("\n");
    }

    if (rc < 0) {
 
        if (errno == 128) {
            LWRN("Git 128 => returning -EAGAIN");
            return -EAGAIN;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            LWRN("Errno: %d => AGAIN", errno);
            return -EAGAIN;
        }

        LWRN("Used FD has error pending! (%d)", (int)rc);
        return (intptr_t)rc;
    }

    return (intptr_t)rc;
}
