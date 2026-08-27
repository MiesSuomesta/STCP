#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/logging/log.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

#include <stcp/stcp.h>

#include "coap_stcp_transport.h"

LOG_MODULE_REGISTER(coap_stcp_transport, LOG_LEVEL_INF);

static int stcp_fd = -1;

int coap_stcp_fd(void)
{
    return stcp_fd;
}

int coap_stcp_connect(void)
{
    struct net_sockaddr_in peer = { 0 };
    int fd;
    int rc;

    if (stcp_fd >= 0) {
        return 0;
    }

    peer.sin_family = NET_AF_INET;
    peer.sin_port = net_htons(CONFIG_STCP_COAP_SERVER_PORT);

    rc = zsock_inet_pton(NET_AF_INET,
                         CONFIG_STCP_COAP_SERVER_IPV4,
                         &peer.sin_addr);
    if (rc != 1) {
        LOG_ERR("Invalid CoAP gateway IPv4 address: %s",
                CONFIG_STCP_COAP_SERVER_IPV4);
        return -EINVAL;
    }

    LOG_INF("CoAP STCP-UDP connect target=%s:%d public=SOCK_STREAM/%d",
            CONFIG_STCP_COAP_SERVER_IPV4,
            CONFIG_STCP_COAP_SERVER_PORT,
            IPPROTO_STCP_UDP);

    /*
     * Canonical STCPv2 public ABI, matching Linux/SDK:
     *   SOCK_STREAM + 253 -> STCP-TCP
     *   SOCK_STREAM + 254 -> STCP-UDP
     *
     * module-v2 maps protocol 254 internally to a UDP carrier.
     */
    fd = zsock_socket(AF_STCP, NET_SOCK_STREAM, IPPROTO_STCP_UDP);
    if (fd < 0) {
        int err = -errno;
        LOG_ERR("CoAP STCP-UDP socket failed errno=%d", errno);
        return err;
    }

    if (zsock_connect(fd,
                      (const struct net_sockaddr *)&peer,
                      sizeof(peer)) < 0) {
        int err = -errno;

        LOG_ERR("CoAP STCP-UDP connect failed target=%s:%d fd=%d errno=%d",
                CONFIG_STCP_COAP_SERVER_IPV4,
                CONFIG_STCP_COAP_SERVER_PORT,
                fd,
                errno);

        (void)zsock_close(fd);
        return err;
    }

    stcp_fd = fd;

    LOG_INF("CoAP STCP-UDP transport connected fd=%d target=%s:%d",
            fd,
            CONFIG_STCP_COAP_SERVER_IPV4,
            CONFIG_STCP_COAP_SERVER_PORT);
    return 0;
}

void coap_stcp_close(void)
{
    if (stcp_fd >= 0) {
        (void)zsock_close(stcp_fd);
        stcp_fd = -1;
    }
}

int coap_stcp_send(const uint8_t *data, size_t len)
{
    size_t sent = 0U;

    if (stcp_fd < 0) {
        return -ENOTCONN;
    }

    while (sent < len) {
        ssize_t n = zsock_send(stcp_fd, data + sent, len - sent, 0);

        if (n < 0) {
            return -errno;
        }
        if (n == 0) {
            return -ECONNRESET;
        }

        sent += (size_t)n;
    }

    return 0;
}

int coap_stcp_recv(uint8_t *data, size_t len)
{
    ssize_t n;

    if (stcp_fd < 0) {
        return -ENOTCONN;
    }

    n = zsock_recv(stcp_fd, data, len, 0);
    if (n < 0) {
        return -errno;
    }

    return (int)n;
}
