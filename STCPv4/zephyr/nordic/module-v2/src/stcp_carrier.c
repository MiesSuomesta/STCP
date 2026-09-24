#include <errno.h>

#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>

#include <stcp/stcp_internal.h>
#include <stcp/stcp_debug.h>

LOG_MODULE_REGISTER(stcp_carrier, CONFIG_STCP_LOG_LEVEL);

int stcp_carrier_open(int socket_type)
{
    int fd;
    int saved_errno;

    errno = 0;
    switch (socket_type) {
    case SOCK_STREAM:
        fd = zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        break;

    case SOCK_DGRAM:
        fd = zsock_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        break;

    default:
        errno = EPROTOTYPE;
        LOG_ERR("carrier socket rejected: unsupported type=%d", socket_type);
        return -1;
    }

    saved_errno = errno;
    STCP_CARRIER_INF("carrier socket return: type=%d fd=%d errno=%d",
            socket_type, fd, saved_errno);

    if (fd < 0) {
        errno = saved_errno;
    }
    return fd;
}

int stcp_carrier_wait_connected(int fd, int timeout_ms)
{
    struct zsock_pollfd pfd = {
        .fd = fd,
        .events = ZSOCK_POLLOUT,
    };
    int rc;
    int error = 0;
    socklen_t error_len = sizeof(error);

    errno = 0;
    rc = zsock_poll(&pfd, 1, timeout_ms);
    STCP_CARRIER_DBG("carrier poll return: fd=%d rc=%d errno=%d revents=0x%x timeout_ms=%d",
            fd, rc, errno, pfd.revents, timeout_ms);

    if (rc == 0) {
        return -ETIMEDOUT;
    }
    if (rc < 0) {
        return -errno;
    }

    errno = 0;
    if (zsock_getsockopt(fd, SOL_SOCKET, SO_ERROR,
                         &error, &error_len) < 0) {
        LOG_ERR("carrier SO_ERROR read failed: fd=%d errno=%d", fd, errno);
        return -errno;
    }

    STCP_CARRIER_DBG("carrier SO_ERROR: fd=%d error=%d", fd, error);
    return error == 0 ? 0 : -error;
}
