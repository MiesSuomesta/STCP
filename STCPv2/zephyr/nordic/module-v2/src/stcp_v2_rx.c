#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <stcp/stcp_v2_internal.h>
#include <stcp/stcp_rust_ffi.h>

LOG_MODULE_REGISTER(stcp_v2_rx, CONFIG_STCP_V2_LOG_LEVEL);

#define RXSTAT_REPORT_MS 500U

static void rxstat_report(struct stcp_v2_socket *sock, const char *reason)
{
    LOG_ERR("RXSTAT reason=%s fd=%d calls=%u eagain=%u bytes=%llu last_errno=%d running=%ld stop=%ld connected=%d",
            reason,
            sock != NULL && sock->carrier != NULL ? sock->carrier->fd : -1,
            sock != NULL ? sock->rxstat_calls : 0U,
            sock != NULL ? sock->rxstat_eagain : 0U,
            (unsigned long long)(sock != NULL ? sock->rxstat_bytes : 0ULL),
            sock != NULL ? sock->rxstat_last_errno : -1,
            sock != NULL ? (long)atomic_get(&sock->rx_running) : -1L,
            sock != NULL ? (long)atomic_get(&sock->rx_stop) : -1L,
            sock != NULL && sock->rust_ctx != NULL ? stcp_rust_is_connected(sock->rust_ctx) : -1);
}

static void rxdiag_log_data(const uint8_t *buffer, ssize_t n)
{
    uint8_t b[8] = {0};
    size_t copy = n > 0 ? MIN((size_t)n, sizeof(b)) : 0U;

    for (size_t i = 0; i < copy; ++i) {
        b[i] = buffer[i];
    }

    LOG_ERR("RXDIAG DATA n=%d first=%02x %02x %02x %02x %02x %02x %02x %02x",
            (int)n,
            b[0], b[1], b[2], b[3], b[4], b[5], b[6], b[7]);
}

static void rx_thread(void *p1, void *p2, void *p3)
{
    struct stcp_v2_socket *sock = p1;
    uint8_t buffer[CONFIG_STCP_V2_RX_BUFFER_SIZE];
    uint32_t last_report_ms = k_uptime_get_32();
    bool first_recv_probe = true;

    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    LOG_ERR("RXPROBE BUILD=20260826-1 sock=%p fd=%d",
            sock,
            sock != NULL && sock->carrier != NULL ? sock->carrier->fd : -1);

    LOG_ERR("RXDIAG THREAD READY sock=%p fd=%d ctx=%p carrier=%p",
            sock,
            sock != NULL && sock->carrier != NULL ? sock->carrier->fd : -1,
            sock != NULL ? sock->rust_ctx : NULL,
            sock != NULL ? sock->carrier : NULL);
    k_sem_give(&sock->rx_ready);
    rxstat_report(sock, "thread-ready");

    while (!atomic_get(&sock->rx_stop) && sock->carrier != NULL && sock->carrier->fd >= 0) {
        ssize_t n;
        int rc = 0;
        int saved_errno;
        uint32_t now_ms;

        sock->rxstat_calls++;
        errno = 0;

        if (sock->socket_type == SOCK_DGRAM) {
            struct sockaddr_in peer = {0};
            socklen_t peer_len = sizeof(peer);

            if (first_recv_probe) {
                LOG_ERR("RXPROBE BEFORE_RECV fd=%d type=dgram", sock->carrier->fd);
            }
            n = zsock_recvfrom(sock->carrier->fd, buffer, sizeof(buffer), 0,
                               (struct sockaddr *)&peer, &peer_len);
            saved_errno = errno;
            if (first_recv_probe) {
                LOG_ERR("RXPROBE AFTER_RECV fd=%d n=%d errno=%d type=dgram",
                        sock->carrier->fd, (int)n, saved_errno);
                first_recv_probe = false;
            }

            if (n > 0) {
                sock->rxstat_bytes += (uint64_t)n;
                sock->rxstat_last_errno = 0;
                rxdiag_log_data(buffer, n);
                LOG_ERR("RXDIAG CORE ENTER ctx=%p n=%d connected=%d",
                        sock->rust_ctx, (int)n,
                        stcp_rust_is_connected(sock->rust_ctx));
                rc = stcp_rust_carrier_receive_from(sock->rust_ctx, buffer, (size_t)n,
                                                    peer.sin_addr.s_addr, peer.sin_port);
                LOG_ERR("RXDIAG CORE RETURN rc=%d connected=%d",
                        rc, stcp_rust_is_connected(sock->rust_ctx));
                rxstat_report(sock, "data");
            }
        } else {
            if (first_recv_probe) {
                LOG_ERR("RXPROBE BEFORE_RECV fd=%d type=stream", sock->carrier->fd);
            }
            n = zsock_recv(sock->carrier->fd, buffer, sizeof(buffer), 0);
            saved_errno = errno;
            if (first_recv_probe) {
                LOG_ERR("RXPROBE AFTER_RECV fd=%d n=%d errno=%d type=stream",
                        sock->carrier->fd, (int)n, saved_errno);
                first_recv_probe = false;
            }

            if (n > 0) {
                sock->rxstat_bytes += (uint64_t)n;
                sock->rxstat_last_errno = 0;
                rxdiag_log_data(buffer, n);
                LOG_ERR("RXDIAG CORE ENTER ctx=%p n=%d connected=%d",
                        sock->rust_ctx, (int)n,
                        stcp_rust_is_connected(sock->rust_ctx));
                rc = stcp_rust_carrier_receive(sock->rust_ctx, buffer, (size_t)n);
                LOG_ERR("RXDIAG CORE RETURN rc=%d connected=%d",
                        rc, stcp_rust_is_connected(sock->rust_ctx));
                rxstat_report(sock, "data");
            }
        }

        if (n > 0) {
            if (rc < 0 && rc != -EAGAIN) {
                LOG_ERR("carrier_receive rc=%d", rc);
            }
            stcp_v2_signal(sock);
            continue;
        }

        if (n == 0) {
            sock->rxstat_last_errno = 0;
            rxstat_report(sock, "eof");
            break;
        }

        sock->rxstat_last_errno = saved_errno;
        if (saved_errno == EAGAIN || saved_errno == EWOULDBLOCK) {
            sock->rxstat_eagain++;
        }

        now_ms = k_uptime_get_32();
        if ((uint32_t)(now_ms - last_report_ms) >= RXSTAT_REPORT_MS) {
            rxstat_report(sock, "periodic");
            last_report_ms = now_ms;
        }

        if (saved_errno == EINTR || saved_errno == EAGAIN || saved_errno == EWOULDBLOCK) {
            k_sleep(K_MSEC(1));
            continue;
        }

        if (!atomic_get(&sock->rx_stop)) {
            LOG_ERR("native carrier recv failed errno=%d", saved_errno);
        }
        rxstat_report(sock, "fatal");
        break;
    }

    rxstat_report(sock, "thread-exit");
    atomic_clear(&sock->rx_running);
    stcp_v2_signal(sock);
}

int stcp_v2_rx_start(struct stcp_v2_socket *sock)
{
    int rc;

    if (sock == NULL || atomic_get(&sock->rx_running)) {
        return 0;
    }

    sock->rxstat_calls = 0U;
    sock->rxstat_eagain = 0U;
    sock->rxstat_bytes = 0ULL;
    sock->rxstat_last_errno = 0;

    atomic_clear(&sock->rx_stop);
    atomic_set(&sock->rx_running, 1);
    k_sem_reset(&sock->rx_ready);

    k_thread_create(&sock->rx_thread, sock->rx_stack,
                    K_KERNEL_STACK_SIZEOF(sock->rx_stack),
                    rx_thread, sock, NULL, NULL,
                    CONFIG_STCP_V2_RX_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&sock->rx_thread, "stcp-v2-rx");

    rc = k_sem_take(&sock->rx_ready, K_SECONDS(1));
    if (rc != 0) {
        LOG_ERR("RXDIAG READY TIMEOUT sock=%p fd=%d rc=%d running=%ld",
                sock,
                sock->carrier != NULL ? sock->carrier->fd : -1,
                rc,
                (long)atomic_get(&sock->rx_running));
        rxstat_report(sock, "ready-timeout");
        atomic_set(&sock->rx_stop, 1);
        if (atomic_get(&sock->rx_running)) {
            k_thread_abort(&sock->rx_thread);
            atomic_clear(&sock->rx_running);
        }
        return -ETIMEDOUT;
    }

    LOG_ERR("RXDIAG START READY sock=%p fd=%d running=%ld",
            sock,
            sock->carrier != NULL ? sock->carrier->fd : -1,
            (long)atomic_get(&sock->rx_running));
    return 0;
}

void stcp_v2_rx_stop(struct stcp_v2_socket *sock)
{
    int join_rc;

    if (sock == NULL || !atomic_get(&sock->rx_running)) {
        return;
    }

    rxstat_report(sock, "stop-enter");
    atomic_set(&sock->rx_stop, 1);

    /*
     * Wake a blocking recv() before waiting for the RX thread.
     *
     * Do NOT use sock->event as a thread-completion primitive here.  That
     * semaphore is also used by handshake/data signalling and may already
     * contain a token.  Consuming such a stale token used to let close()
     * continue while rx_thread was still running, after which rust_ctx and
     * carrier could be released underneath it.
     */
    if (sock->carrier != NULL && sock->carrier->owns_fd && sock->carrier->fd >= 0) {
        int shutdown_rc;
        int saved_errno;

        errno = 0;
        shutdown_rc = zsock_shutdown(sock->carrier->fd, ZSOCK_SHUT_RDWR);
        saved_errno = errno;
        LOG_ERR("LIFECYCLE RX SHUTDOWN fd=%d rc=%d errno=%d",
                sock->carrier->fd, shutdown_rc, saved_errno);
    }

    LOG_ERR("LIFECYCLE RX JOIN ENTER sock=%p running=%ld",
            sock, (long)atomic_get(&sock->rx_running));
    join_rc = k_thread_join(&sock->rx_thread, K_SECONDS(1));
    LOG_ERR("LIFECYCLE RX JOIN RETURN sock=%p rc=%d running=%ld",
            sock, join_rc, (long)atomic_get(&sock->rx_running));

    if (join_rc != 0) {
        /* Hard fallback: make teardown deterministic before freeing ctx/fd. */
        LOG_ERR("LIFECYCLE RX ABORT sock=%p join_rc=%d", sock, join_rc);
        k_thread_abort(&sock->rx_thread);
        (void)k_thread_join(&sock->rx_thread, K_FOREVER);
        atomic_clear(&sock->rx_running);
    }

    /* The normal thread exit clears rx_running itself. */
    if (atomic_get(&sock->rx_running)) {
        LOG_ERR("LIFECYCLE RX JOINED BUT RUNNING sock=%p; forcing clear", sock);
        atomic_clear(&sock->rx_running);
    }

    /* No stale event token is allowed to leak into any later close/read path. */
    k_sem_reset(&sock->event);
    rxstat_report(sock, "stop-exit");
}
