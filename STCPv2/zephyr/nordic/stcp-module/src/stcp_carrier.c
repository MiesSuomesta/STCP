#include <errno.h>

#include <zephyr/net/socket.h>

#include <stcp/stcp_internal.h>

int stcp_carrier_open(int socket_type)
{
    switch (socket_type) {
    case SOCK_STREAM:
        return zsock_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    case SOCK_DGRAM:
        return zsock_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    default:
        errno = EPROTOTYPE;
        return -1;
    }
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

    rc = zsock_poll(&pfd, 1, timeout_ms);
    if (rc == 0) {
        return -ETIMEDOUT;
    }
    if (rc < 0) {
        return -errno;
    }

    if (zsock_getsockopt(fd, SOL_SOCKET, SO_ERROR,
                         &error, &error_len) < 0) {
        return -errno;
    }

    return error == 0 ? 0 : -error;
}
