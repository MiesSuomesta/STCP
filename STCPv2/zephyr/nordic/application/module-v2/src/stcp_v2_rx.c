#include <errno.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <stcp/stcp_v2_internal.h>
#include <stcp/stcp_rust_ffi.h>

LOG_MODULE_REGISTER(stcp_v2_rx, CONFIG_STCP_V2_LOG_LEVEL);

static void rx_thread(void *p1, void *p2, void *p3)
{
    struct stcp_v2_socket *sock = p1;
    uint8_t buffer[CONFIG_STCP_V2_RX_BUFFER_SIZE];
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    /* Startup barrier: connect_socket() must not start the Rust handshake
     * until this RX thread is actually executing. */
    k_sem_give(&sock->rx_ready);

    while (!atomic_get(&sock->rx_stop) && sock->carrier != NULL && sock->carrier->fd >= 0) {
        ssize_t n;
        int rc;

        if (sock->socket_type == SOCK_DGRAM) {
            struct sockaddr_in peer = {0};
            socklen_t peer_len = sizeof(peer);
            n = zsock_recvfrom(sock->carrier->fd, buffer, sizeof(buffer), 0,
                               (struct sockaddr *)&peer, &peer_len);
            if (n > 0) {
                rc = stcp_rust_carrier_receive_from(sock->rust_ctx, buffer, (size_t)n,
                                                    peer.sin_addr.s_addr, peer.sin_port);
            } else {
                rc = 0;
            }
        } else {
            n = zsock_recv(sock->carrier->fd, buffer, sizeof(buffer), 0);
            if (n > 0) {
                rc = stcp_rust_carrier_receive(sock->rust_ctx, buffer, (size_t)n);
            } else {
                rc = 0;
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
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) {
            k_sleep(K_MSEC(1));
            continue;
        }
        if (!atomic_get(&sock->rx_stop)) {
            LOG_ERR("native carrier recv failed errno=%d", errno);
        }
        break;
    }

    atomic_clear(&sock->rx_running);
    stcp_v2_signal(sock);
}

int stcp_v2_rx_start(struct stcp_v2_socket *sock)
{
    int rc;

    if (sock == NULL) {
        return -EINVAL;
    }
    if (atomic_get(&sock->rx_running)) {
        return 0;
    }

    atomic_clear(&sock->rx_stop);
    k_sem_reset(&sock->rx_ready);
    atomic_set(&sock->rx_running, 1);

    k_thread_create(&sock->rx_thread, sock->rx_stack,
                    K_KERNEL_STACK_SIZEOF(sock->rx_stack),
                    rx_thread, sock, NULL, NULL,
                    CONFIG_STCP_V2_RX_PRIORITY, 0, K_NO_WAIT);
    k_thread_name_set(&sock->rx_thread, "stcp-v2-rx");

    rc = k_sem_take(&sock->rx_ready, K_MSEC(1000));
    if (rc != 0) {
        LOG_ERR("RX thread startup timeout rc=%d fd=%d", rc,
                sock->carrier != NULL ? sock->carrier->fd : -1);
        atomic_set(&sock->rx_stop, 1);
        if (atomic_get(&sock->rx_running)) {
            k_thread_abort(&sock->rx_thread);
            atomic_clear(&sock->rx_running);
        }
        return -ETIMEDOUT;
    }

    LOG_INF("RX thread ready fd=%d",
            sock->carrier != NULL ? sock->carrier->fd : -1);
    return 0;
}

void stcp_v2_rx_stop(struct stcp_v2_socket *sock)
{
    if (sock == NULL || !atomic_get(&sock->rx_running)) {
        return;
    }
    atomic_set(&sock->rx_stop, 1);
    if (sock->carrier != NULL && sock->carrier->owns_fd && sock->carrier->fd >= 0) {
        (void)zsock_shutdown(sock->carrier->fd, ZSOCK_SHUT_RDWR);
    }
    (void)k_sem_take(&sock->event, K_MSEC(250));
    if (atomic_get(&sock->rx_running)) {
        k_thread_abort(&sock->rx_thread);
        atomic_clear(&sock->rx_running);
    }
}
