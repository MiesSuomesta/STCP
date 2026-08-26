#include <string.h>
#include <zephyr/kernel.h>
#include <stcp/stcp_v2_internal.h>

static struct stcp_v2_socket sockets[CONFIG_STCP_V2_MAX_SOCKETS];
static K_MUTEX_DEFINE(sockets_lock);

struct stcp_v2_socket *stcp_v2_socket_alloc(void)
{
    struct stcp_v2_socket *ret = NULL;

    k_mutex_lock(&sockets_lock, K_FOREVER);
    for (size_t i = 0; i < ARRAY_SIZE(sockets); ++i) {
        if (sockets[i].used) {
            continue;
        }
        memset(&sockets[i], 0, sizeof(sockets[i]));
        sockets[i].used = true;
        sockets[i].fd = -1;
        k_sem_init(&sockets[i].event, 0, 1);
        k_sem_init(&sockets[i].rx_ready, 0, 1);
        atomic_clear(&sockets[i].rx_running);
        atomic_clear(&sockets[i].rx_stop);
        ret = &sockets[i];
        break;
    }
    k_mutex_unlock(&sockets_lock);
    return ret;
}

void stcp_v2_socket_free(struct stcp_v2_socket *sock)
{
    if (sock == NULL) {
        return;
    }
    k_mutex_lock(&sockets_lock, K_FOREVER);
    memset(sock, 0, sizeof(*sock));
    sock->fd = -1;
    k_mutex_unlock(&sockets_lock);
}

void stcp_v2_signal(struct stcp_v2_socket *sock)
{
    if (sock != NULL) {
        k_sem_give(&sock->event);
    }
}
