#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <stcp/stcp_v2_internal.h>
#include <stcp/stcp_rust_ffi.h>

LOG_MODULE_REGISTER(stcp_v2_rx, CONFIG_STCP_V2_LOG_LEVEL);

static void rx_thread(void *p1, void *p2, void *p3)
{
    struct stcp_v2_socket *sock = p1;
    uint8_t buffer[CONFIG_STCP_V2_RX_BUFFER_SIZE];

    bool rx_ready_signaled = false;

    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    while (!atomic_get(&sock->rx_stop) && sock->carrier != NULL && sock->carrier->fd >= 0) {
        ssize_t n;
        int rc = 0;
        int saved_errno;

        errno = 0;

        if (sock->carrier->socket_type == SOCK_DGRAM) {
            struct sockaddr_in peer = {0};
            socklen_t peer_len = sizeof(peer);
            LOG_ERR("RXDIAG RECVFROM ENTER sock=%p carrier=%p fd=%d ctx=%p",
                    sock, sock->carrier, sock->carrier->fd, sock->rust_ctx);

            if (!rx_ready_signaled) {
                LOG_ERR("RXDIAG RECVFROM ARM sock=%p fd=%d ctx=%p",
                        sock, sock->carrier->fd, sock->rust_ctx);
                rx_ready_signaled = true;
                k_sem_give(&sock->rx_ready);
            }

            n = zsock_recvfrom(sock->carrier->fd, buffer, sizeof(buffer), 0,
                               (struct sockaddr *)&peer, &peer_len);
            saved_errno = errno;

            LOG_ERR("RXDIAG RECVFROM RETURN sock=%p fd=%d n=%d errno=%d peer_port=%u",
                    sock, sock->carrier->fd, (int)n,
                    n < 0 ? saved_errno : 0,
                    (unsigned int)ntohs(peer.sin_port));

            if (n > 0) {
                {
                    size_t unused = 0U;
                    int stack_rc = k_thread_stack_space_get(&sock->rx_thread, &unused);

                    LOG_ERR("RXSTACK BEFORE_RUST sock=%p n=%d rc=%d unused=%u total=%u used=%u",
                            sock, (int)n, stack_rc,
                            (unsigned int)unused,
                            (unsigned int)K_KERNEL_STACK_SIZEOF(sock->rx_stack),
                            stack_rc == 0 &&
                            unused <= K_KERNEL_STACK_SIZEOF(sock->rx_stack)
                                ? (unsigned int)(K_KERNEL_STACK_SIZEOF(sock->rx_stack) - unused)
                                : 0U);
                }

                LOG_ERR("RXDIAG RUST ENTER ctx=%p n=%d peer_port=%u",
                        sock->rust_ctx, (int)n,
                        (unsigned int)ntohs(peer.sin_port));

                rc = stcp_rust_carrier_receive_from(sock->rust_ctx, buffer, (size_t)n,
                                                    peer.sin_addr.s_addr, peer.sin_port);

                {
                    size_t unused = 0U;
                    int stack_rc = k_thread_stack_space_get(&sock->rx_thread, &unused);

                    LOG_ERR("RXSTACK AFTER_RUST sock=%p n=%d rc=%d unused=%u total=%u used=%u",
                            sock, (int)n, stack_rc,
                            (unsigned int)unused,
                            (unsigned int)K_KERNEL_STACK_SIZEOF(sock->rx_stack),
                            stack_rc == 0 &&
                            unused <= K_KERNEL_STACK_SIZEOF(sock->rx_stack)
                                ? (unsigned int)(K_KERNEL_STACK_SIZEOF(sock->rx_stack) - unused)
                                : 0U);
                }

                LOG_ERR("RXDIAG RUST RETURN ctx=%p n=%d rc=%d connected=%d",
                        sock->rust_ctx, (int)n, rc,
                        stcp_rust_is_connected(sock->rust_ctx));
            }
        } else {
            if (!rx_ready_signaled) {
                LOG_ERR("RXDIAG RECV ARM sock=%p fd=%d ctx=%p",
                        sock, sock->carrier->fd, sock->rust_ctx);
                rx_ready_signaled = true;
                k_sem_give(&sock->rx_ready);
            }

            n = zsock_recv(sock->carrier->fd, buffer, sizeof(buffer), 0);
            saved_errno = errno;

            if (n > 0) {
                rc = stcp_rust_carrier_receive(sock->rust_ctx, buffer, (size_t)n);
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
            break;
        }

        if (saved_errno == EINTR || saved_errno == EAGAIN || saved_errno == EWOULDBLOCK) {
            k_sleep(K_MSEC(1));
            continue;
        }

        if (!atomic_get(&sock->rx_stop)) {
            LOG_ERR("native carrier recv failed errno=%d", saved_errno);
        }
        break;
    }
    atomic_clear(&sock->rx_running);
    stcp_v2_signal(sock);
}

int stcp_v2_rx_start(struct stcp_v2_socket *sock)
{
    int rc;

    if (sock == NULL || atomic_get(&sock->rx_running)) {
        return 0;
    }


    atomic_clear(&sock->rx_stop);
    atomic_set(&sock->rx_running, 1);
    k_sem_reset(&sock->rx_ready);

    k_thread_create(&sock->rx_thread, sock->rx_stack,
                    K_KERNEL_STACK_SIZEOF(sock->rx_stack),
                    rx_thread, sock, NULL, NULL,
                    CONFIG_STCP_V2_RX_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&sock->rx_thread, "stcp-v2-rx");

    rc = k_sem_take(&sock->rx_ready, K_MSEC(1000));
    if (rc != 0) {
        LOG_ERR("RXREADY TIMEOUT sock=%p fd=%d rc=%d",
                sock,
                sock->carrier != NULL ? sock->carrier->fd : -1,
                rc);

        atomic_set(&sock->rx_stop, 1);
        k_thread_abort(&sock->rx_thread);
        atomic_clear(&sock->rx_running);
        return -ETIMEDOUT;
    }

    LOG_ERR("RXREADY CONFIRMED sock=%p fd=%d running=%ld",
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
}
