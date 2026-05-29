/*
 * Copyright (c) 2018 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/** @file mqtt_transport_socket_tcp.h
 *
 * @brief Internal functions to handle transport over TCP socket.
 */



#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>

#include <stcp/stcp_struct.h>
#include <stcp/stcp_api.h>
#include <stcp/stcp_api_internal.h>

#include <stcp/stcp_transport_api.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_socket.h>
#include <errno.h>

#include <stcp/debug.h>

#define STCP_MQTT_READ_DEBUG        1
#define STCP_MQTT_WRITE_DEBUG       1

#if CONFIG_MQTT_LIB_STCP

#include <stcp/mqtt_stcp_stats.h>

extern struct mqtt_stcp_stats g_mqtt_stcp_stats;

#define RECV_RETRY_DELAY_MS                 5
#define STCP_CONNECTION_WAIT_TIMEOUT_MS    (5*1000)

#define TRACE_CALL(clientPtr) \
    do {                                                                                                            \
        LDBG("MQTT LOW: Transport function %s called with client %p", __func__, clientPtr);                         \
        LDBG(                                                                                                       \
            "MQTT cfg: client_id=%p client_size=%d rx_buf=%p rx_size=%u tx_buf=%p tx_size=%u broker=%p evt_cb=%p transport=%d ver=%d clean=%d",    \
            clientPtr->client_id.utf8,                                                                              \
            clientPtr->client_id.size,                                                                              \
            clientPtr->rx_buf,                                                                                      \
            clientPtr->rx_buf_size,                                                                                 \
            clientPtr->tx_buf,                                                                                      \
            clientPtr->tx_buf_size,                                                                                 \
            clientPtr->broker,                                                                                      \
            clientPtr->evt_cb,                                                                                      \
            clientPtr->transport.type,                                                                              \
            clientPtr->protocol_version,                                                                            \
            clientPtr->clean_session                                                                                \
        );                                                                                                          \
        stcp_dump_bt();                                                                                             \
    } while(0)

/* Initialize STCP context and connect to broker */
int mqtt_client_stcp_connect(struct mqtt_client *client)
{
    TRACE_CALL(client);

    if (!client || !client->broker) {
        LERR("Invalid client or broker");
        return -1;
    }

    LDBG("Getting broker..");
    const struct sockaddr *broker = client->broker;

    struct stcp_api *api = NULL;

    int fd = zsock_socket(broker->sa_family, SOCK_STREAM, IPPROTO_TCP);

    if (fd < 0) {
        LDBG("MQTT: Got no sock for api %p, errno=%d, rc=%d!", api, errno, fd);
        return -EBADFD;
    }

    LDBG("Doing api init with FD: %d", fd);
    int rc = stcp_api_init_with_fd(&api, fd);
    LDBGBIG("Done api init, rc: %d => %p", rc, api);

    if (!api) {
        LWRNBIG("MQTT: OOM, no mana...");
        if (fd >= 0) {
            LDBG("Closing FD %d", fd);
            zsock_close(fd);
        }
        return -ENOMEM;
    }

    // ÄLÄ SIIRRÄ TÄSTÄ NÄITÄ POIS!
    // MQTT reffi
    // HETI MQTT omistukseen, tämä pitää referenssin 
    // hengissä koko MQTT elinkaaren läpi.
    LDBGBIG("Doing api init ref (MQTT REF)...");
    STCP_REF_COUNT_GET(api->ctx, "the MQTT reference", LDBG("No access panic."); k_panic(); );

    if (rc < 0) {
        LDBG("Doing error: %d", rc);
        if (api != NULL) {

            STCP_REF_COUNT_PUT(
                api->ctx,
                "the MQTT reference"
            );

            LDBG("Closing API...");
            stcp_api_close(api);
        } else {
            if (fd >= 0) {
                LDBG("Closing FD %d", fd);
                zsock_close(fd);
            }
        }
        LERR("Returnign rc: %d", rc);
        return rc;
    }

    // ÄLÄ SIIRRÄ TÄSTÄ NÄITÄ POIS!
    LDBG("MQTT: Setting api %p for client %p..", api, client);
    client->transport.stcp.stcp_api_instance = api;
    LDBG("MQTT: Putting fd on API handle: %d ..", stcp_api_get_fd(api));
    client->transport.stcp.sock = stcp_api_get_fd(api);

    LDBG("MQTT: API init: %p", api);
    if (api && api->ctx) {
        stcp_debug_dump_stcp_ctx(api->ctx);
    }
    LDBG("Got API %p with FD: %d, rc: %d", api, fd, rc);

    LDBG("After error check");

#if defined(CONFIG_SOCKS)
    if (client->transport.proxy.addrlen != 0) {
        rc = setsockopt(fd, SOL_SOCKET, SO_SOCKS5,
                        &client->transport.proxy.addr,
                        client->transport.proxy.addrlen);
        if (rc < 0) {
            MDBG("Dec of life...");
            goto error;
        }
    }
#endif

    size_t peer_addr_size = (broker->sa_family == AF_INET) ?
                             sizeof(struct sockaddr_in) :
                             sizeof(struct sockaddr_in6);
    LDBG("Doing STCP connect....");
    rc = stcp_api_connect(api, client->broker, peer_addr_size);
    LDBG("Connect to broker, rc=%d errno=%d", rc, errno);
    if (rc < 0) {
        if (rc != -EINPROGRESS && rc != -EAGAIN) {
            LDBG("Connect error, rc=%d errno=%d", rc, errno);

            STCP_REF_COUNT_PUT(
                api->ctx,
                "the MQTT reference"
            );

            stcp_api_close(api);
            return rc;
        }
    }

    LDBG("MQTT: API %p Waiting for connection to complete hanshake...", api);  
    rc = stcp_api_wait_until_stcp_handshake_is_done(
        api,
        STCP_CONNECTION_WAIT_TIMEOUT_MS
    );

    if (rc < 0) {
        LDBG("MQTT: API %p connection hanshake error %d", api, rc); 

        STCP_REF_COUNT_PUT(
            api->ctx,
            "the MQTT reference"
        );
        
        stcp_api_close(api);

        return rc;
    }

    LDBG("MQTT: Setting API %p alive..", api);
    atomic_set(&api->alive, 1); // Force to be alive..


    LDBG("After connect..");  
	g_mqtt_stcp_stats.connects++;
    LDBG("All done, OK path!");
    return 0;
}

/* Send raw buffer over STCP */
int mqtt_client_stcp_write(struct mqtt_client *client,
                           const uint8_t *data,
                           uint32_t len)
{
    TRACE_CALL(client);
#if STCP_MQTT_WRITE_DEBUG
    LDBG("Params: client: %p, data: %p, len: %d", client, data, len);
#endif
    struct stcp_api *api = client->transport.stcp.stcp_api_instance;

    if (!api) return -EINVAL;

    if ( !stcp_api_is_alive(api) ) {
        LWRNBIG("API %p is dead!", api);
        return -EPROTO;
    }

    STCP_REF_COUNT_GET(api->ctx, "@ MQTT write", return -EACCES; );

    g_mqtt_stcp_stats.tx_calls++;

#if STCP_MQTT_WRITE_DEBUG
    MDBGBIG("@ WRITE[%d bytes]: %s",len, data);
    STCP_LOG_HEX("Before write, buffer contents", data, len);
#endif

    size_t offset = 0;

    while (offset < len) {
#if STCP_MQTT_WRITE_DEBUG
        LDBG("@API send: %p, %p, %d", api, data + offset, len - offset);
        LDBGBIG("MQTT LOW: BEFORE stcp_api_send len=%u", len - offset);
#endif

        int ret = stcp_api_send(api, data + offset, len - offset, 0);

#if STCP_MQTT_WRITE_DEBUG
        LDBGBIG("MQTT LOW: AFTER stcp_api_send rc=%d errno=%d", ret, errno);
#endif

        if (ret == -EAGAIN) {

            g_mqtt_stcp_stats.tx_eagain++;
            k_sleep(K_MSEC(5));
            continue;
        }

        if (ret < 0) {
            STCP_REF_COUNT_PUT(api->ctx, "@ MQTT write error");
            return ret;
        }

        offset += ret;
        g_mqtt_stcp_stats.tx_bytes += ret;
    }

    STCP_REF_COUNT_PUT(api->ctx, "@ End of MQTT write");
    return offset;
}


int mqtt_client_stcp_write_msg(struct mqtt_client *client,
                               const struct net_msghdr *message)
{
    TRACE_CALL(client);

    if (!client || !message) {
        LERR("STCP WRITE MSG: No client or message!");
        return -EINVAL;
    }

    struct stcp_api *api = client->transport.stcp.stcp_api_instance;
    if (!api) {
        LERR("STCP WRITE_MSG: api NULL");
        return -EINVAL;
    }

    if (!stcp_api_is_alive(api)) {
        LWRNBIG("STCP WRITE_MSG: API %p is dead!", api);
        return -EPROTO;
    }

    STCP_REF_COUNT_GET(api->ctx, "@ MQTT Write msg", return -EACCES; );

    size_t total_len = 0;

    /* Laske kokonaispituus */
    for (size_t i = 0; i < message->msg_iovlen; i++) {
        total_len += message->msg_iov[i].iov_len;
    }

#if STCP_MQTT_WRITE_DEBUG
    MDBGBIG("@ WRITE_MSG total=%d bytes", total_len);
#endif

    size_t sent_total = 0;

    /* Käydään iovec läpi yksi kerrallaan */
    for (size_t i = 0; i < message->msg_iovlen; i++) {

        const uint8_t *buf = message->msg_iov[i].iov_base;
        size_t len = message->msg_iov[i].iov_len;

        size_t offset = 0;

        while (offset < len) {

            int ret = stcp_api_send(api,
                                    buf + offset,
                                    len - offset,
                                    0);

            if (ret == -EAGAIN) {
                g_mqtt_stcp_stats.tx_eagain++;
                k_sleep(K_MSEC(5));
                continue;
            }

            if (ret < 0) {
                LERR("STCP WRITE_MSG error: %d", ret);
                STCP_REF_COUNT_PUT(api->ctx, "@ MQTT write msg error");
                return ret;
            }

            offset += ret;
            sent_total += ret;
            g_mqtt_stcp_stats.tx_bytes += ret;
        }
    }

    g_mqtt_stcp_stats.tx_calls++;

    STCP_REF_COUNT_PUT(api->ctx, "@ End of MQTT write msg");

    return sent_total;
}

/* Receive data over STCP */
int mqtt_client_stcp_read(struct mqtt_client *client,
                          uint8_t *data,
                          uint32_t buflen,
                          bool shall_block)
{
    TRACE_CALL(client);

    struct stcp_api *api = client->transport.stcp.stcp_api_instance;
    if (!api) return -EINVAL;

    if ( !stcp_api_is_alive(api) ) {
        LWRNBIG("API %p is dead!", api);
        return -EPROTO;
    }

    STCP_REF_COUNT_GET(api->ctx, "@ MQTT Read", return -EACCES; );

    int flags = 0;

    if (!shall_block) {
        flags = ZSOCK_MSG_DONTWAIT;
    }

    g_mqtt_stcp_stats.rx_calls++;
#if STCP_MQTT_READ_DEBUG
    LDBGBIG("@ READ[%d bytes max] block: %d, read to %p", buflen, shall_block, data);
#endif
    int ret = stcp_api_recv(
        api,
        data,
        buflen,
        flags
    );

#if STCP_MQTT_READ_DEBUG
    LDBG(
        "MQTT READ ret=%d errno=%d",
        ret,
        errno
    );
#endif

    if (ret > 0) {

#if STCP_MQTT_READ_DEBUG
        STCP_LOG_HEX(
            "MQTT RX RAW",
            data,
            ret
        );
#endif

        g_mqtt_stcp_stats.rx_bytes += ret;
    }

    if (ret < 0) {

#if STCP_MQTT_READ_DEBUG
        LDBGBIG(
            "@ AFTER READ: Error %d",
            ret
        );
#endif

        if (ret == -EAGAIN) {
            g_mqtt_stcp_stats.rx_eagain++;
        }

        errno = -ret;

        STCP_REF_COUNT_PUT(
            api->ctx,
            "@ MQTT Read error"
        );

        return ret;
    }

    
    if (ret == 0) {

#if STCP_MQTT_READ_DEBUG
        LWRN("MQTT transport EOF");
#endif

        STCP_REF_COUNT_PUT(
            api->ctx,
            "@ MQTT EOF"
        );

        return 0;
    }

#if STCP_MQTT_READ_DEBUG
    LDBGBIG("@ AFTER READ[%d/%d bytes]", ret, buflen);
#endif

    STCP_REF_COUNT_PUT(api->ctx, "@ End of MQTT Read");
    return ret;
}

/* Close STCP session */
int mqtt_client_stcp_disconnect(struct mqtt_client *client)
{
    TRACE_CALL(client);

    if (!client) {
        LERR("MQTT: No client");
        return -EINVAL;
    }
    struct stcp_api *api = client->transport.stcp.stcp_api_instance;
    if (!api) {
        LERR("MQTT: No api");
        return 0;
    }

    // NYT API:n omistaja on ON VIELÄ MQTT, 
    // Tuon sisällä tapahtuu kaikki ..

    // Ota referenssi talteen tässä
    struct stcp_ctx *ctx = api->ctx;

    if (!ctx) {
        LERR("MQTT: No api ctx");
        return 0;
    }

    // ...ja estä käyttö
    if (client) {
        client->transport.stcp.stcp_api_instance = NULL;
    }

    int rc = stcp_api_close(api);

    LERR("MQTT: API %p close, refcount after close: %d", api, atomic_get(&ctx->refcnt));

    // Poistetaan MQTT:n omistus tässä, JÄLKEEN closen,
    // sen takia, että close ehtii ottamaan referenssin
    // ja tämä putti ei tuhoa kontekstia. Nyt omistus 
    // siirtyy STCP:lle tämän jälkeen:
    STCP_REF_COUNT_PUT(ctx, "the MQTT reference");

    // Fire and forget.....
    LDBG("Client disconnected...");
    client->transport.stcp.stcp_api_instance = NULL;
    return rc;
}

#endif
