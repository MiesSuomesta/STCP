#include <errno.h>
#include <string.h>
#include <zephyr/net/socket.h>
#include <stcp/stcp_internal.h>

static int neg_errno(void)
{
    return errno > 0 ? -errno : -EIO;
}

int stcp_carrier_init(struct stcp_carrier *carrier, int protocol)
{
    int type;
    int ipproto;

    if (carrier == NULL) {
        return -EINVAL;
    }

    memset(carrier, 0, sizeof(*carrier));
    carrier->fd = -1;

    switch (protocol) {
    case STCP_PROTO_DEFAULT:
    case STCP_PROTO_TCP:
        carrier->kind = STCP_CARRIER_TCP;
        type = SOCK_STREAM;
        ipproto = IPPROTO_TCP;
        break;
    case STCP_PROTO_UDP:
        carrier->kind = STCP_CARRIER_UDP;
        type = SOCK_DGRAM;
        ipproto = IPPROTO_UDP;
        break;
    default:
        return -EPROTONOSUPPORT;
    }

    /* AF_INET is intentional: this is the hidden transport below AF_STCP. */
    carrier->fd = zsock_socket(AF_INET, type, ipproto);
    if (carrier->fd < 0) {
        return neg_errno();
    }

    return 0;
}

void stcp_carrier_close(struct stcp_carrier *carrier)
{
    if (carrier != NULL && carrier->fd >= 0) {
        (void)zsock_close(carrier->fd);
        carrier->fd = -1;
    }
}

int stcp_carrier_bind(struct stcp_carrier *carrier, const struct sockaddr_in *addr)
{
    if (carrier == NULL || addr == NULL || carrier->fd < 0) {
        return -EINVAL;
    }
    if (zsock_bind(carrier->fd, (const struct sockaddr *)addr, sizeof(*addr)) < 0) {
        return neg_errno();
    }
    carrier->bound = true;
    return 0;
}

int stcp_carrier_connect(struct stcp_carrier *carrier, const struct sockaddr_in *addr)
{
    struct zsock_pollfd pfd;
    int socket_error = 0;
    socklen_t socket_error_len = sizeof(socket_error);
    int ret;

    if (carrier == NULL || addr == NULL || carrier->fd < 0) {
        return -EINVAL;
    }

    ret = zsock_connect(carrier->fd, (const struct sockaddr *)addr, sizeof(*addr));
    if (ret == 0) {
        carrier->connected = true;
        return 0;
    }

    if (errno != EINPROGRESS && errno != EALREADY && errno != EAGAIN) {
        return neg_errno();
    }

    pfd.fd = carrier->fd;
    pfd.events = ZSOCK_POLLOUT;
    pfd.revents = 0;

    ret = zsock_poll(&pfd, 1, CONFIG_STCP_CONNECT_TIMEOUT_MS);
    if (ret == 0) {
        return -ETIMEDOUT;
    }
    if (ret < 0) {
        return neg_errno();
    }

    if (zsock_getsockopt(carrier->fd, SOL_SOCKET, SO_ERROR,
                         &socket_error, &socket_error_len) < 0) {
        return neg_errno();
    }
    if (socket_error != 0) {
        return -socket_error;
    }

    carrier->connected = true;
    return 0;
}

int stcp_carrier_listen(struct stcp_carrier *carrier, int backlog)
{
    if (carrier == NULL || carrier->fd < 0 || backlog < 1) {
        return -EINVAL;
    }
    if (carrier->kind != STCP_CARRIER_TCP) {
        return -EOPNOTSUPP;
    }
    if (zsock_listen(carrier->fd, backlog) < 0) {
        return neg_errno();
    }
    carrier->listening = true;
    return 0;
}

int stcp_carrier_accept(struct stcp_carrier *listener, struct stcp_carrier *child,
                        struct sockaddr_in *peer, socklen_t *peer_len)
{
    int fd;

    if (listener == NULL || child == NULL || listener->fd < 0) {
        return -EINVAL;
    }
    if (listener->kind != STCP_CARRIER_TCP || !listener->listening) {
        return -EINVAL;
    }

    fd = zsock_accept(listener->fd, (struct sockaddr *)peer, peer_len);
    if (fd < 0) {
        return neg_errno();
    }

    memset(child, 0, sizeof(*child));
    child->kind = STCP_CARRIER_TCP;
    child->fd = fd;
    child->connected = true;
    return 0;
}

ssize_t stcp_carrier_send(struct stcp_carrier *carrier, const void *buf,
                          size_t len, int flags)
{
    ssize_t ret;
    if (carrier == NULL || carrier->fd < 0 || (buf == NULL && len != 0)) {
        return -EINVAL;
    }
    ret = zsock_send(carrier->fd, buf, len, flags);
    return ret < 0 ? neg_errno() : ret;
}

ssize_t stcp_carrier_recv(struct stcp_carrier *carrier, void *buf,
                          size_t len, int flags)
{
    ssize_t ret;
    if (carrier == NULL || carrier->fd < 0 || (buf == NULL && len != 0)) {
        return -EINVAL;
    }
    ret = zsock_recv(carrier->fd, buf, len, flags);
    return ret < 0 ? neg_errno() : ret;
}

int stcp_carrier_shutdown(struct stcp_carrier *carrier, int how)
{
    if (carrier == NULL || carrier->fd < 0) {
        return -EINVAL;
    }
    if (zsock_shutdown(carrier->fd, how) < 0) {
        return neg_errno();
    }
    return 0;
}

int stcp_carrier_getsockname(struct stcp_carrier *carrier,
                             struct sockaddr_in *addr, socklen_t *len)
{
    if (carrier == NULL || addr == NULL || len == NULL || carrier->fd < 0) {
        return -EINVAL;
    }
    if (zsock_getsockname(carrier->fd, (struct sockaddr *)addr, len) < 0) {
        return neg_errno();
    }
    return 0;
}

int stcp_carrier_getpeername(struct stcp_carrier *carrier,
                             struct sockaddr_in *addr, socklen_t *len)
{
    if (carrier == NULL || addr == NULL || len == NULL || carrier->fd < 0) {
        return -EINVAL;
    }
    if (zsock_getpeername(carrier->fd, (struct sockaddr *)addr, len) < 0) {
        return neg_errno();
    }
    return 0;
}

int stcp_carrier_getsockopt(struct stcp_carrier *carrier, int level, int optname,
                            void *optval, socklen_t *optlen)
{
    if (carrier == NULL || carrier->fd < 0 || optval == NULL || optlen == NULL) {
        return -EINVAL;
    }
    if (zsock_getsockopt(carrier->fd, level, optname, optval, optlen) < 0) {
        return neg_errno();
    }
    return 0;
}

int stcp_carrier_setsockopt(struct stcp_carrier *carrier, int level, int optname,
                            const void *optval, socklen_t optlen)
{
    if (carrier == NULL || carrier->fd < 0 || optval == NULL) {
        return -EINVAL;
    }
    if (zsock_setsockopt(carrier->fd, level, optname, optval, optlen) < 0) {
        return neg_errno();
    }
    return 0;
}
