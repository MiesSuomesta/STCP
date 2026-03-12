/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/** @file mqtt_transport_socket_tcp.h
 *
 * @brief Internal functions to handle transport over TCP socket.
 */



#include <errno.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>


#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>

#include <stcp/stcp_transport_api.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_socket.h>

#include <stcp/debug.h>

#include <stcp/mqtt_stcp_stats.h>

extern struct mqtt_stcp_stats g_mqtt_stcp_stats;


#define RECV_RETRY_DELAY_MS 5
#define MDBG(fmt, ...) LDBG("[STCP / MQTT] " fmt,  ##__VA_ARGS__)

/* Initialize STCP context and connect to broker */
int mqtt_client_stcp_connect(struct mqtt_client *client)
{
    MDBG("Connecting...");

    if (!client || !client->broker) {
        LERR("Invalid client or broker");
        return -EINVAL;
    }

    const struct sockaddr *broker = client->broker;
    struct stcp_api *api = NULL;
    int fd = zsock_socket(broker->sa_family, SOCK_STREAM, IPPROTO_TCP);

    MDBG("Got FD: %d", fd);
    if (fd < 0) {
        return fd;
    }

    int rc = stcp_api_init_with_fd(&api, fd);
    MDBG("Got API %p with FD: %d, rc: %d", api, fd, rc);
    if (rc < 0) {
        if (api != NULL) stcp_api_close(api);
        return rc;
    }

    client->transport.stcp.sock = fd;
    client->transport.stcp.stcp_api_instance = api;

#if defined(CONFIG_SOCKS)
    if (client->transport.proxy.addrlen != 0) {
        rc = setsockopt(fd, SOL_SOCKET, SO_SOCKS5,
                        &client->transport.proxy.addr,
                        client->transport.proxy.addrlen);
        if (rc < 0) {
            goto error;
        }
    }
#endif

    size_t peer_addr_size = (broker->sa_family == AF_INET) ?
                             sizeof(struct sockaddr_in) :
                             sizeof(struct sockaddr_in6);

    rc = stcp_api_connect(api, client->broker, peer_addr_size);
    MDBG("Connect to broker, rc=%d", rc);
    if (rc < 0 && errno != EISCONN) {
        goto error;
    }

	g_mqtt_stcp_stats.connects++;
    return 0;

error:
    if (api != NULL) stcp_api_close(api);
    return rc;
}

/* Send raw buffer over STCP */
int mqtt_client_stcp_write(struct mqtt_client *client,
                           const uint8_t *data,
                           uint32_t len)
{
    struct stcp_api *api = client->transport.stcp.stcp_api_instance;

    g_mqtt_stcp_stats.tx_calls++;

    size_t offset = 0;

    while (offset < len) {

        int ret = stcp_api_send(api, data + offset, len - offset, 0);

        if (ret == -EAGAIN) {

            g_mqtt_stcp_stats.tx_eagain++;
            k_sleep(K_MSEC(5));
            continue;
        }

        if (ret < 0) {
            return ret;
        }

        offset += ret;
        g_mqtt_stcp_stats.tx_bytes += ret;
    }

    return offset;
}

/* Send scatter-gather message */
int mqtt_client_stcp_write_msg(struct mqtt_client *client,
                               const struct msghdr *message)
{
    if (!client || !message) return -EINVAL;

    struct stcp_api *api = client->transport.stcp.stcp_api_instance;
    if (!api) return -EINVAL;

    size_t total_len = 0;
    for (size_t i = 0; i < message->msg_iovlen; i++) {
        total_len += message->msg_iov[i].iov_len;
    }

    size_t offset = 0;
    while (offset < total_len) {
        int ret = stcp_api_sendmsg(api, message);
        if (ret < 0) return ret;
        offset += ret;

        /* Adjust msghdr for remaining data */
        for (size_t i = 0; i < message->msg_iovlen; i++) {
            if ((size_t)ret < message->msg_iov[i].iov_len) {
                message->msg_iov[i].iov_base =
                    (uint8_t *)message->msg_iov[i].iov_base + ret;
                message->msg_iov[i].iov_len -= ret;
                break;
            }
            ret -= message->msg_iov[i].iov_len;
            message->msg_iov[i].iov_len = 0;
        }
    }

    g_mqtt_stcp_stats.tx_bytes += total_len;
	return 0;
}

/* Receive data over STCP */
int mqtt_client_stcp_read(struct mqtt_client *client,
                          uint8_t *data,
                          uint32_t buflen,
                          bool shall_block)
{
    struct stcp_api *api = client->transport.stcp.stcp_api_instance;

    int flags = 0;

    if (!shall_block)
        flags |= ZSOCK_MSG_DONTWAIT;

    g_mqtt_stcp_stats.rx_calls++;

    int ret = stcp_api_recv(api, data, buflen, flags);

    if (ret == -EAGAIN) {
        g_mqtt_stcp_stats.rx_eagain++;
        return -EAGAIN;
    }

    if (ret >= 0)
        g_mqtt_stcp_stats.rx_bytes += ret;

    return ret;
}

/* Close STCP session */
int mqtt_client_stcp_disconnect(struct mqtt_client *client)
{
    if (!client) return -EINVAL;

    struct stcp_api *api = client->transport.stcp.stcp_api_instance;
    if (!api) return 0;

    int rc = stcp_api_close(api);
    client->transport.stcp.stcp_api_instance = NULL;

    return rc;
}