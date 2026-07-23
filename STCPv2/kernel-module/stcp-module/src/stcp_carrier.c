#include <errno.h>
#include <zephyr/net/socket.h>
#include <stcp/stcp_internal.h>
int stcp_carrier_open(int protocol)
{
    int type = (protocol == STCP_PROTO_UDP) ? SOCK_DGRAM : SOCK_STREAM;
    int proto = (protocol == STCP_PROTO_UDP) ? IPPROTO_UDP : IPPROTO_TCP;
    return zsock_socket(AF_INET, type, proto);
}
int stcp_carrier_wait_connected(int fd, int timeout_ms)
{
    struct zsock_pollfd pfd = { .fd = fd, .events = ZSOCK_POLLOUT };
    int rc = zsock_poll(&pfd, 1, timeout_ms);
    if (rc == 0) return -ETIMEDOUT;
    if (rc < 0) return -errno;
    int err = 0;
    socklen_t len = sizeof(err);
    if (zsock_getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len) < 0) return -errno;
    return err ? -err : 0;
}
