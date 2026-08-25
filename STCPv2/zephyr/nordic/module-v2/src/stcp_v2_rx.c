#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <stcp/stcp_v2_internal.h>
#include <stcp/stcp_rust_ffi.h>

LOG_MODULE_REGISTER(stcp_v2_rx, CONFIG_STCP_V2_LOG_LEVEL);

#define STCP_RX_DIAG_DUMP_LIMIT 8
#define STCP_RX_DIAG_IDLE_LOG_EVERY 250

static void rx_thread(void *p1, void *p2, void *p3)
{
    struct stcp_v2_socket *sock = p1;
    uint8_t buffer[CONFIG_STCP_V2_RX_BUFFER_SIZE];
    unsigned int recv_calls = 0;
    unsigned int data_reads = 0;
    unsigned int idle_reads = 0;

    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    LOG_INF("RXDIAG thread ENTER sock=%p app_fd=%d carrier=%p native_fd=%d "
            "type=%d ctx=%p stop=%ld running=%ld",
            sock,
            sock != NULL ? sock->fd : -1,
            sock != NULL ? sock->carrier : NULL,
            (sock != NULL && sock->carrier != NULL) ? sock->carrier->fd : -1,
            sock != NULL ? sock->socket_type : -1,
            sock != NULL ? sock->rust_ctx : NULL,
            sock != NULL ? (long)atomic_get(&sock->rx_stop) : -1L,
            sock != NULL ? (long)atomic_get(&sock->rx_running) : -1L);

    while (!atomic_get(&sock->rx_stop) &&
           sock->carrier != NULL &&
           sock->carrier->fd >= 0) {
        ssize_t n;
        int rc;
        int saved_errno;

        recv_calls++;

        /*
         * Clear errno before every native receive so the diagnostic always
         * reports the errno belonging to this call, not a stale value.
         */
        errno = 0;

        if (sock->socket_type == SOCK_DGRAM) {
            struct sockaddr_in peer = {0};
            socklen_t peer_len = sizeof(peer);

            n = zsock_recvfrom(sock->carrier->fd,
                               buffer,
                               sizeof(buffer),
                               0,
                               (struct sockaddr *)&peer,
                               &peer_len);
            saved_errno = errno;

            if (n > 0) {
                rc = stcp_rust_carrier_receive_from(sock->rust_ctx,
                                                    buffer,
                                                    (size_t)n,
                                                    peer.sin_addr.s_addr,
                                                    peer.sin_port);
            } else {
                rc = 0;
            }
        } else {
            n = zsock_recv(sock->carrier->fd,
                           buffer,
                           sizeof(buffer),
                           0);
            saved_errno = errno;

            if (n > 0) {
                rc = stcp_rust_carrier_receive(sock->rust_ctx,
                                               buffer,
                                               (size_t)n);
            } else {
                rc = 0;
            }
        }

        if (n > 0) {
            data_reads++;

            LOG_INF("RXDIAG DATA call=%u read=%u native_fd=%d n=%d "
                    "core_rc=%d stop=%ld",
                    recv_calls,
                    data_reads,
                    sock->carrier->fd,
                    (int)n,
                    rc,
                    (long)atomic_get(&sock->rx_stop));

            if (data_reads <= STCP_RX_DIAG_DUMP_LIMIT) {
                LOG_HEXDUMP_INF(buffer,
                                (size_t)n,
                                "RXDIAG native RX payload");
            }

            if (rc < 0 && rc != -EAGAIN) {
                LOG_ERR("RXDIAG carrier_receive FAILED call=%u n=%d rc=%d",
                        recv_calls, (int)n, rc);
            }

            stcp_v2_signal(sock);
            continue;
        }

        if (n == 0) {
            LOG_WRN("RXDIAG PEER CLOSED call=%u native_fd=%d stop=%ld",
                    recv_calls,
                    sock->carrier->fd,
                    (long)atomic_get(&sock->rx_stop));
            break;
        }

        if (saved_errno == EINTR ||
            saved_errno == EAGAIN ||
            saved_errno == EWOULDBLOCK) {
            idle_reads++;

            if (idle_reads == 1 ||
                (idle_reads % STCP_RX_DIAG_IDLE_LOG_EVERY) == 0) {
                LOG_INF("RXDIAG idle call=%u idle=%u native_fd=%d errno=%d "
                        "stop=%ld running=%ld",
                        recv_calls,
                        idle_reads,
                        sock->carrier->fd,
                        saved_errno,
                        (long)atomic_get(&sock->rx_stop),
                        (long)atomic_get(&sock->rx_running));
            }

            k_sleep(K_MSEC(1));
            continue;
        }

        if (!atomic_get(&sock->rx_stop)) {
            LOG_ERR("RXDIAG native recv FAILED call=%u native_fd=%d n=%d "
                    "errno=%d stop=%ld carrier=%p ctx=%p",
                    recv_calls,
                    sock->carrier->fd,
                    (int)n,
                    saved_errno,
                    (long)atomic_get(&sock->rx_stop),
                    sock->carrier,
                    sock->rust_ctx);
        }
        break;
    }

    LOG_INF("RXDIAG thread EXIT sock=%p app_fd=%d recv_calls=%u "
            "data_reads=%u idle_reads=%u stop=%ld carrier=%p native_fd=%d",
            sock,
            sock != NULL ? sock->fd : -1,
            recv_calls,
            data_reads,
            idle_reads,
            sock != NULL ? (long)atomic_get(&sock->rx_stop) : -1L,
            sock != NULL ? sock->carrier : NULL,
            (sock != NULL && sock->carrier != NULL) ? sock->carrier->fd : -1);

    atomic_clear(&sock->rx_running);
    stcp_v2_signal(sock);
}

int stcp_v2_rx_start(struct stcp_v2_socket *sock)
{
    k_tid_t tid;

    if (sock == NULL) {
        LOG_ERR("RXDIAG start called with NULL socket");
        return -EINVAL;
    }

    LOG_INF("RXDIAG start ENTER sock=%p app_fd=%d carrier=%p native_fd=%d "
            "running=%ld stop=%ld ctx=%p",
            sock,
            sock->fd,
            sock->carrier,
            sock->carrier != NULL ? sock->carrier->fd : -1,
            (long)atomic_get(&sock->rx_running),
            (long)atomic_get(&sock->rx_stop),
            sock->rust_ctx);

    if (atomic_get(&sock->rx_running)) {
        LOG_WRN("RXDIAG start skipped: already running sock=%p app_fd=%d",
                sock, sock->fd);
        return 0;
    }

    atomic_clear(&sock->rx_stop);
    atomic_set(&sock->rx_running, 1);

    tid = k_thread_create(&sock->rx_thread,
                          sock->rx_stack,
                          K_KERNEL_STACK_SIZEOF(sock->rx_stack),
                          rx_thread,
                          sock,
                          NULL,
                          NULL,
                          CONFIG_STCP_V2_RX_PRIORITY,
                          0,
                          K_NO_WAIT);

    LOG_INF("RXDIAG k_thread_create returned tid=%p sock=%p native_fd=%d "
            "prio=%d stack=%u",
            tid,
            sock,
            sock->carrier != NULL ? sock->carrier->fd : -1,
            CONFIG_STCP_V2_RX_PRIORITY,
            (unsigned int)K_KERNEL_STACK_SIZEOF(sock->rx_stack));

    k_thread_name_set(&sock->rx_thread, "stcp-v2-rx");

    LOG_INF("RXDIAG start EXIT sock=%p app_fd=%d native_fd=%d running=%ld",
            sock,
            sock->fd,
            sock->carrier != NULL ? sock->carrier->fd : -1,
            (long)atomic_get(&sock->rx_running));

    return 0;
}

void stcp_v2_rx_stop(struct stcp_v2_socket *sock)
{
    if (sock == NULL) {
        return;
    }

    LOG_INF("RXDIAG stop ENTER sock=%p app_fd=%d carrier=%p native_fd=%d "
            "running=%ld stop=%ld",
            sock,
            sock->fd,
            sock->carrier,
            sock->carrier != NULL ? sock->carrier->fd : -1,
            (long)atomic_get(&sock->rx_running),
            (long)atomic_get(&sock->rx_stop));

    if (!atomic_get(&sock->rx_running)) {
        LOG_INF("RXDIAG stop: RX not running");
        return;
    }

    atomic_set(&sock->rx_stop, 1);

    if (sock->carrier != NULL &&
        sock->carrier->owns_fd &&
        sock->carrier->fd >= 0) {
        int shutdown_rc;

        errno = 0;
        shutdown_rc = zsock_shutdown(sock->carrier->fd, ZSOCK_SHUT_RDWR);

        LOG_INF("RXDIAG shutdown native_fd=%d rc=%d errno=%d",
                sock->carrier->fd,
                shutdown_rc,
                errno);
    }

    (void)k_sem_take(&sock->event, K_MSEC(250));

    if (atomic_get(&sock->rx_running)) {
        LOG_WRN("RXDIAG RX still running after stop wait; aborting thread");
        k_thread_abort(&sock->rx_thread);
        atomic_clear(&sock->rx_running);
    }

    LOG_INF("RXDIAG stop EXIT sock=%p app_fd=%d running=%ld",
            sock,
            sock->fd,
            (long)atomic_get(&sock->rx_running));
}
