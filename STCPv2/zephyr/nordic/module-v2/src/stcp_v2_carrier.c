#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stcp/stcp_v2_internal.h>

struct stcp_v2_carrier *stcp_v2_carrier_open(int socket_type)
{
    struct stcp_v2_carrier *carrier = k_calloc(1, sizeof(*carrier));
    if (carrier == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    carrier->socket_type = socket_type;
    carrier->owns_fd = true;
    carrier->fd = zsock_socket(AF_INET, socket_type,
                               socket_type == SOCK_DGRAM ? IPPROTO_UDP : IPPROTO_TCP);
    if (carrier->fd < 0) {
        k_free(carrier);
        return NULL;
    }
    k_mutex_init(&carrier->tx_lock);
    return carrier;
}

struct stcp_v2_carrier *stcp_v2_carrier_udp_child(struct stcp_v2_carrier *listener,
                                                   uint32_t peer_addr,
                                                   uint16_t peer_port)
{
    if (listener == NULL || listener->fd < 0) {
        return NULL;
    }
    struct stcp_v2_carrier *child = k_calloc(1, sizeof(*child));
    if (child == NULL) {
        return NULL;
    }
    child->fd = listener->fd;
    child->socket_type = SOCK_DGRAM;
    child->owns_fd = false;
    child->peer_valid = true;
    child->peer.sin_family = AF_INET;
    child->peer.sin_addr.s_addr = peer_addr;
    child->peer.sin_port = peer_port;
    k_mutex_init(&child->tx_lock);
    return child;
}

void stcp_v2_carrier_free(struct stcp_v2_carrier *carrier)
{
    if (carrier == NULL) {
        return;
    }
    if (carrier->owns_fd && carrier->fd >= 0) {
        (void)zsock_close(carrier->fd);
        carrier->fd = -1;
    }
    k_free(carrier);
}

ssize_t stcp_v2_carrier_send_wire(struct stcp_v2_carrier *carrier,
                                  const uint8_t *data, size_t len, int flags)
{
    ssize_t rc;
    if (carrier == NULL || carrier->fd < 0) {
        return -ENOTCONN;
    }

    k_mutex_lock(&carrier->tx_lock, K_FOREVER);
    if (carrier->socket_type == SOCK_DGRAM && carrier->peer_valid) {
        rc = zsock_sendto(carrier->fd, data, len, flags,
                          (const struct sockaddr *)&carrier->peer,
                          sizeof(carrier->peer));
    } else {
        rc = zsock_send(carrier->fd, data, len, flags);
    }
    k_mutex_unlock(&carrier->tx_lock);

    return rc < 0 ? -errno : rc;
}
