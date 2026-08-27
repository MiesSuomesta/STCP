#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/logging/log.h>
#include <zephyr/net/mqtt.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

#include <stcp/stcp.h>

#include "mqtt_stcp_transport.h"

LOG_MODULE_REGISTER(mqtt_stcp_transport, LOG_LEVEL_INF);

static struct mqtt_stcp_transport_data *transport_data(struct mqtt_client *client)
{
    return (struct mqtt_stcp_transport_data *)
        client->transport.custom_transport_data;
}

static const struct mqtt_stcp_transport_data *
transport_data_const(const struct mqtt_client *client)
{
    return (const struct mqtt_stcp_transport_data *)
        client->transport.custom_transport_data;
}

int mqtt_stcp_poll_fd(const struct mqtt_client *client)
{
    const struct mqtt_stcp_transport_data *data = transport_data_const(client);

    return data != NULL ? data->fd : -1;
}

int mqtt_client_custom_transport_connect(struct mqtt_client *client)
{
    struct mqtt_stcp_transport_data *data = transport_data(client);
    const struct net_sockaddr *broker =
        (const struct net_sockaddr *)client->broker;
    net_socklen_t broker_len;
    int fd;

    if (data == NULL || broker == NULL) {
        return -EINVAL;
    }

    if (broker->sa_family != NET_AF_INET) {
        return -EAFNOSUPPORT;
    }

    broker_len = sizeof(struct net_sockaddr_in);

    fd = zsock_socket(AF_STCP, NET_SOCK_STREAM, IPPROTO_STCP);
    if (fd < 0) {
        return -errno;
    }

    if (zsock_connect(fd, broker, broker_len) < 0) {
        int err = -errno;

        (void)zsock_close(fd);
        return err;
    }

    data->fd = fd;

    LOG_INF("MQTT custom transport connected over STCP fd=%d", fd);
    return 0;
}

int mqtt_client_custom_transport_write(struct mqtt_client *client,
                                       const uint8_t *data,
                                       uint32_t datalen)
{
    struct mqtt_stcp_transport_data *transport = transport_data(client);
    uint32_t sent = 0U;

    if (transport == NULL || transport->fd < 0) {
        return -ENOTCONN;
    }

    while (sent < datalen) {
        ssize_t n = zsock_send(transport->fd,
                               data + sent,
                               datalen - sent,
                               0);
        if (n < 0) {
            return -errno;
        }

        if (n == 0) {
            return -ECONNRESET;
        }

        sent += (uint32_t)n;
    }

    return 0;
}

int mqtt_client_custom_transport_write_msg(struct mqtt_client *client,
                                           const struct net_msghdr *message)
{
    struct mqtt_stcp_transport_data *transport = transport_data(client);

    if (transport == NULL || transport->fd < 0) {
        return -ENOTCONN;
    }

    /*
     * Avoid any POSIX msghdr/iovec assumptions here.  Zephyr MQTT gives us
     * net_msghdr + net_iovec; send each vector through the same STCP stream.
     */
    for (size_t i = 0U; i < message->msg_iovlen; ++i) {
        const uint8_t *buf =
            (const uint8_t *)message->msg_iov[i].iov_base;
        size_t left = message->msg_iov[i].iov_len;

        while (left > 0U) {
            ssize_t n = zsock_send(transport->fd, buf, left, 0);

            if (n < 0) {
                return -errno;
            }

            if (n == 0) {
                return -ECONNRESET;
            }

            buf += (size_t)n;
            left -= (size_t)n;
        }
    }

    return 0;
}

int mqtt_client_custom_transport_read(struct mqtt_client *client,
                                      uint8_t *data,
                                      uint32_t buflen,
                                      bool shall_block)
{
    struct mqtt_stcp_transport_data *transport = transport_data(client);
    int flags = shall_block ? 0 : ZSOCK_MSG_DONTWAIT;
    ssize_t n;

    if (transport == NULL || transport->fd < 0) {
        return -ENOTCONN;
    }

    n = zsock_recv(transport->fd, data, buflen, flags);
    if (n < 0) {
        return -errno;
    }

    return (int)n;
}

int mqtt_client_custom_transport_disconnect(struct mqtt_client *client)
{
    struct mqtt_stcp_transport_data *data = transport_data(client);

    if (data == NULL || data->fd < 0) {
        return 0;
    }

    (void)zsock_close(data->fd);
    data->fd = -1;

    LOG_INF("MQTT custom STCP transport disconnected");
    return 0;
}
