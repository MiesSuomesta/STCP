#include <errno.h>
#include <stdint.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/random/random.h>

#include <stcp/stcp.h>

#include "mqtt_codec.h"
#if defined(CONFIG_NRF_MODEM_LIB)
#include "stcp_lte_transport.h"
#endif

LOG_MODULE_REGISTER(mqtt_proxy, LOG_LEVEL_INF);

#define MQTT_PROXY_HOST       "lja.fi"
#define MQTT_PROXY_PORT       "7777"
#define MQTT_UPLINK_TOPIC     "stcp2/nrf9151/up"
#define MQTT_DOWNLINK_TOPIC   "stcp2/nrf9151/down"
#define MQTT_KEEPALIVE_SEC    60
#define MQTT_RX_TIMEOUT_MS    1000
#define MQTT_PING_INTERVAL_MS 30000
#define MQTT_RETRY_MIN_MS     1000U
#define MQTT_RETRY_MAX_MS     60000U

static int connect_stcp(void)
{
    const struct zsock_addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct zsock_addrinfo *result = NULL;
    int fd;
    int rc;

    rc = zsock_getaddrinfo(MQTT_PROXY_HOST, MQTT_PROXY_PORT,
                           &hints, &result);
    if (rc != 0 || result == NULL) {
        return -EHOSTUNREACH;
    }

    fd = zsock_socket(AF_STCP, SOCK_STREAM, IPPROTO_STCP);
    if (fd < 0) {
        rc = -errno;
        zsock_freeaddrinfo(result);
        return rc;
    }

#if defined(CONFIG_NRF_MODEM_LIB)
    rc = stcp_lte_transport_bind_socket(fd);
    if (rc < 0) {
        zsock_close(fd);
        zsock_freeaddrinfo(result);
        return rc;
    }
#endif

    if (zsock_connect(fd, result->ai_addr, result->ai_addrlen) < 0) {
        rc = -errno;
        zsock_close(fd);
        zsock_freeaddrinfo(result);
        return rc;
    }

    zsock_freeaddrinfo(result);
    return fd;
}

static int setup_mqtt_session(int fd)
{
    char client_id[40];
    int rc;

    snprintk(client_id, sizeof(client_id), "nrf9151-%08x",
             sys_rand32_get());

    rc = mqtt_send_connect(fd, client_id, MQTT_KEEPALIVE_SEC);
    if (rc == 0) {
        rc = mqtt_wait_connack(fd, 15000);
    }
    if (rc == 0) {
        rc = mqtt_send_subscribe(fd, MQTT_DOWNLINK_TOPIC, 1);
    }

    return rc;
}

static int handle_publish(int fd, uint8_t flags,
                          const uint8_t *packet, size_t packet_len,
                          uint16_t *next_packet_id)
{
    size_t topic_len;
    size_t pos;
    size_t payload_len;
    uint16_t received_packet_id = 0;
    int rc;

    if (packet_len < 2) {
        return -EBADMSG;
    }

    topic_len = ((size_t)packet[0] << 8) | packet[1];
    pos = 2 + topic_len;
    if (pos > packet_len) {
        return -EBADMSG;
    }

    if ((flags & 0x06U) != 0U) {
        if (pos + 2 > packet_len) {
            return -EBADMSG;
        }
        received_packet_id = ((uint16_t)packet[pos] << 8) | packet[pos + 1];
        pos += 2;
    }

    payload_len = packet_len - pos;
    LOG_INF("MQTT downlink bytes=%u", (unsigned int)payload_len);

    rc = mqtt_send_publish_qos1(fd, MQTT_UPLINK_TOPIC,
                                packet + pos, payload_len,
                                (*next_packet_id)++);
    if (rc < 0) {
        return rc;
    }

    if (received_packet_id != 0U) {
        rc = mqtt_send_puback(fd, received_packet_id);
    }

    return rc;
}

static int run_connected_session(int fd)
{
    uint16_t next_packet_id = 2;
    int64_t last_tx = k_uptime_get();

    for (;;) {
        uint8_t packet_type;
        uint8_t flags;
        uint8_t packet[512];
        size_t packet_len;
        int rc;

        rc = mqtt_recv_packet(fd, &packet_type, &flags,
                              packet, sizeof(packet), &packet_len,
                              MQTT_RX_TIMEOUT_MS);
        if (rc == -ETIMEDOUT) {
            if (k_uptime_get() - last_tx > MQTT_PING_INTERVAL_MS) {
                rc = mqtt_send_pingreq(fd);
                if (rc < 0) {
                    return rc;
                }
                last_tx = k_uptime_get();
            }
            continue;
        }
        if (rc < 0) {
            return rc;
        }

        if (packet_type == 3U) {
            rc = handle_publish(fd, flags, packet, packet_len,
                                &next_packet_id);
            if (rc < 0) {
                return rc;
            }
            last_tx = k_uptime_get();
        }
    }
}

int mqtt_proxy_run(void)
{
    uint32_t retry_ms = MQTT_RETRY_MIN_MS;

    for (;;) {
        int fd = connect_stcp();
        int rc;

        if (fd < 0) {
            LOG_WRN("STCP connect failed: %d", fd);
            k_sleep(K_MSEC(retry_ms));
            retry_ms = MIN(retry_ms * 2U, MQTT_RETRY_MAX_MS);
            continue;
        }

        retry_ms = MQTT_RETRY_MIN_MS;
        rc = setup_mqtt_session(fd);
        if (rc == 0) {
            LOG_INF("MQTT over STCP connected to %s:%s",
                    MQTT_PROXY_HOST, MQTT_PROXY_PORT);
            rc = run_connected_session(fd);
        } else {
            LOG_ERR("MQTT setup failed: %d", rc);
        }

        LOG_WRN("MQTT/STCP disconnected: %d", rc);
        zsock_close(fd);
    }
}
