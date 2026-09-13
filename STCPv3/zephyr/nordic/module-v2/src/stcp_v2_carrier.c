#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <stcp/stcp_v2_internal.h>

LOG_MODULE_REGISTER(stcp_v2_carrier, CONFIG_STCP_V2_LOG_LEVEL);

struct stcp_v2_carrier *stcp_v2_carrier_open(int socket_type)
{
    struct stcp_v2_carrier *carrier = k_calloc(1, sizeof(*carrier));

    if (carrier == NULL) {
        errno = ENOMEM;
        return NULL;
    }

    carrier->socket_type = socket_type;
    carrier->owns_fd = true;

    carrier->fd = zsock_socket(AF_INET,
                               socket_type,
                               socket_type == SOCK_DGRAM
                                   ? IPPROTO_UDP
                                   : IPPROTO_TCP);

    if (carrier->fd < 0) {
        LOG_ERR("LIFECYCLE NATIVE OPEN FAIL carrier=%p type=%d errno=%d",
                carrier, socket_type, errno);
        k_free(carrier);
        return NULL;
    }

    LOG_ERR("LIFECYCLE NATIVE OPEN carrier=%p type=%d fd=%d",
            carrier, socket_type, carrier->fd);

    k_mutex_init(&carrier->tx_lock);

    return carrier;
}

struct stcp_v2_carrier *
stcp_v2_carrier_udp_child(struct stcp_v2_carrier *listener,
                          uint32_t peer_addr,
                          uint16_t peer_port)
{
    struct stcp_v2_carrier *child;

    if (listener == NULL || listener->fd < 0) {
        return NULL;
    }

    child = k_calloc(1, sizeof(*child));
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
        int old_fd = carrier->fd;
        int close_rc;
        int saved_errno;

        LOG_ERR("LIFECYCLE NATIVE CLOSE ENTER carrier=%p fd=%d",
                carrier, old_fd);

        errno = 0;
        close_rc = zsock_close(old_fd);
        saved_errno = errno;

        LOG_ERR("LIFECYCLE NATIVE CLOSE RETURN carrier=%p fd=%d rc=%d errno=%d",
                carrier, old_fd, close_rc, saved_errno);

        carrier->fd = -1;
    }

    LOG_ERR("LIFECYCLE CARRIER KFREE carrier=%p", carrier);

    k_free(carrier);
}

ssize_t stcp_v2_carrier_send_wire(struct stcp_v2_carrier *carrier,
                                  const uint8_t *data,
                                  size_t len,
                                  int flags)
{
    ssize_t rc;
    int saved_errno;

    if (carrier == NULL || carrier->fd < 0) {
        return -ENOTCONN;
    }

    k_mutex_lock(&carrier->tx_lock, K_FOREVER);

    LOG_ERR("TXDIAG ENTER carrier=%p fd=%d type=%d peer_valid=%d len=%u flags=%d",
            carrier,
            carrier->fd,
            carrier->socket_type,
            carrier->peer_valid,
            (unsigned int)len,
            flags);

    errno = 0;

    if (carrier->socket_type == SOCK_DGRAM && carrier->peer_valid) {
        rc = zsock_sendto(carrier->fd,
                          data,
                          len,
                          flags,
                          (const struct sockaddr *)&carrier->peer,
                          sizeof(carrier->peer));
    } else {
        rc = zsock_send(carrier->fd,
                        data,
                        len,
                        flags);
    }

    saved_errno = errno;

    LOG_ERR("TXDIAG RETURN carrier=%p fd=%d type=%d peer_valid=%d len=%u rc=%d errno=%d",
            carrier,
            carrier->fd,
            carrier->socket_type,
            carrier->peer_valid,
            (unsigned int)len,
            (int)rc,
            saved_errno);

    k_mutex_unlock(&carrier->tx_lock);

    if (rc < 0) {
        return -saved_errno;
    }

    return rc;
}

