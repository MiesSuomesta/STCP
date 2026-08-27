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
    printk("ZR 4 thread_entry sock=%p thread=%p stack=%p fd=%d ctx=%p\n",
           sock,
           sock != NULL ? &sock->rx_thread : NULL,
           sock != NULL ? sock->rx_stack : NULL,
           sock != NULL && sock->carrier != NULL ? sock->carrier->fd : -1,
           sock != NULL ? sock->rust_ctx : NULL);

    ARG_UNUSED(p2);
    ARG_UNUSED(p3);
    printk("ZR 5 ready_give sock=%p\n", sock);
    k_sem_give(&sock->rx_ready);

    while (!atomic_get(&sock->rx_stop) && sock->carrier != NULL && sock->carrier->fd >= 0) {
        ssize_t n;
        int rc = 0;
        int saved_errno;

        errno = 0;

        if (sock->socket_type == SOCK_DGRAM) {
            struct sockaddr_in peer = {0};
            socklen_t peer_len = sizeof(peer);
            n = zsock_recvfrom(sock->carrier->fd, buffer, sizeof(buffer), 0,
                               (struct sockaddr *)&peer, &peer_len);
            saved_errno = errno;

            if (n > 0) {
                rc = stcp_rust_carrier_receive_from(sock->rust_ctx, buffer, (size_t)n,
                                                    peer.sin_addr.s_addr, peer.sin_port);
            }
        } else {
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

    printk("ZR 1 rx_start_enter sock=%p fd=%d running=%ld stop=%ld\n",
           sock,
           sock != NULL && sock->carrier != NULL ? sock->carrier->fd : -1,
           sock != NULL ? (long)atomic_get(&sock->rx_running) : -1L,
           sock != NULL ? (long)atomic_get(&sock->rx_stop) : -1L);

    if (sock == NULL || atomic_get(&sock->rx_running)) {
        return 0;
    }


    atomic_clear(&sock->rx_stop);
    atomic_set(&sock->rx_running, 1);
    k_sem_reset(&sock->rx_ready);

    printk("ZR 2 before_thread_create sock=%p thread=%p stack=%p size=%zu align32=%lu\n",
           sock,
           &sock->rx_thread,
           sock->rx_stack,
           (size_t)K_KERNEL_STACK_SIZEOF(sock->rx_stack),
           (unsigned long)((uintptr_t)sock->rx_stack & 31U));

    k_tid_t zr_tid = k_thread_create(&sock->rx_thread, sock->rx_stack,
                                     K_KERNEL_STACK_SIZEOF(sock->rx_stack),
                                     rx_thread, sock, NULL, NULL,
                                     CONFIG_STCP_V2_RX_PRIORITY, 0, K_NO_WAIT);

    printk("ZR 3 after_thread_create tid=%p running=%ld\n",
           zr_tid, (long)atomic_get(&sock->rx_running));
    k_thread_name_set(&sock->rx_thread, "stcp-v2-rx");

    rc = k_sem_take(&sock->rx_ready, K_SECONDS(1));
    printk("ZR 6 ready_wait_return rc=%d running=%ld\n",
           rc, (long)atomic_get(&sock->rx_running));
    if (rc != 0) {
        atomic_set(&sock->rx_stop, 1);
        if (atomic_get(&sock->rx_running)) {
            k_thread_abort(&sock->rx_thread);
            atomic_clear(&sock->rx_running);
        }
        return -ETIMEDOUT;
    }
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
