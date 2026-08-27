#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/net/mqtt.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/net/socket.h>

#include "mqtt_stcp_transport.h"

LOG_MODULE_REGISTER(stcp_mqtt_app, LOG_LEVEL_INF);

static struct mqtt_client client;

static struct mqtt_stcp_transport_data transport = {
    .fd = -1,
};

static struct net_sockaddr_in broker;

static uint8_t rx_buffer[1024];
static uint8_t tx_buffer[1024];

static bool connected;

static void mqtt_evt_handler(struct mqtt_client *const client,
                             const struct mqtt_evt *evt)
{
    ARG_UNUSED(client);

    switch (evt->type) {
    case MQTT_EVT_CONNACK:
        if (evt->result == 0 && evt->param.connack.return_code == 0) {
            connected = true;
            LOG_INF("MQTT CONNACK OK");
        } else {
            LOG_ERR("MQTT CONNACK failed result=%d code=%u",
                    evt->result,
                    evt->param.connack.return_code);
        }
        break;

    case MQTT_EVT_DISCONNECT:
        connected = false;
        LOG_WRN("MQTT disconnected result=%d", evt->result);
        break;

    case MQTT_EVT_PUBACK:
        LOG_INF("MQTT PUBACK id=%u", evt->param.puback.message_id);
        break;

    case MQTT_EVT_PINGRESP:
        LOG_DBG("MQTT PINGRESP");
        break;

    default:
        break;
    }
}

static int broker_init(void)
{
    memset(&broker, 0, sizeof(broker));

    broker.sin_family = NET_AF_INET;
    broker.sin_port = net_htons(CONFIG_STCP_MQTT_BROKER_PORT);

    if (zsock_inet_pton(NET_AF_INET,
                        CONFIG_STCP_MQTT_BROKER_IPV4,
                        &broker.sin_addr) != 1) {
        return -EINVAL;
    }

    return 0;
}

static void client_init(void)
{
    mqtt_client_init(&client);

    client.broker = &broker;
    client.evt_cb = mqtt_evt_handler;

    client.client_id.utf8 = (uint8_t *)CONFIG_STCP_MQTT_CLIENT_ID;
    client.client_id.size = strlen(CONFIG_STCP_MQTT_CLIENT_ID);

    client.password = NULL;
    client.user_name = NULL;

    client.protocol_version = MQTT_VERSION_3_1_1;

    client.rx_buf = rx_buffer;
    client.rx_buf_size = sizeof(rx_buffer);
    client.tx_buf = tx_buffer;
    client.tx_buf_size = sizeof(tx_buffer);

    client.transport.type = MQTT_TRANSPORT_CUSTOM;
    client.transport.custom_transport_data = &transport;
}

/*
 * AF_STCP does not currently provide poll() readiness semantics that the
 * generic Zephyr MQTT sample loop expects.  The custom transport read path is
 * non-blocking when mqtt_input() asks for non-blocking input, so drive the
 * MQTT parser directly and treat -EAGAIN as "no packet yet".
 */
static int mqtt_service_once(void)
{
    int rc;

    rc = mqtt_input(&client);
    if (rc < 0 && rc != -EAGAIN) {
        return rc;
    }

    rc = mqtt_live(&client);
    if (rc < 0 && rc != -EAGAIN) {
        return rc;
    }

    return 0;
}

static int service_until_connected(int timeout_ms)
{
    int64_t deadline = k_uptime_get() + timeout_ms;

    while (!connected && k_uptime_get() < deadline) {
        int rc = mqtt_service_once();

        if (rc < 0) {
            LOG_ERR("MQTT service during CONNACK wait failed: %d", rc);
            return rc;
        }

        if (connected) {
            return 0;
        }

        k_sleep(K_MSEC(20));
    }

    return connected ? 0 : -ETIMEDOUT;
}

static int publish_once(uint32_t sequence)
{
    char payload[256];
    struct mqtt_publish_param param;
    int len;

    len = snprintk(payload,
                   sizeof(payload),
                   "%s seq=%u uptime_ms=%lld",
                   CONFIG_STCP_MQTT_PAYLOAD,
                   sequence,
                   (long long)k_uptime_get());

    if (len < 0) {
        return len;
    }

    if ((size_t)len >= sizeof(payload)) {
        len = sizeof(payload) - 1;
    }

    memset(&param, 0, sizeof(param));

    param.message.topic.qos = MQTT_QOS_0_AT_MOST_ONCE;
    param.message.topic.topic.utf8 =
        (uint8_t *)CONFIG_STCP_MQTT_TOPIC;
    param.message.topic.topic.size =
        strlen(CONFIG_STCP_MQTT_TOPIC);

    param.message.payload.data = (uint8_t *)payload;
    param.message.payload.len = (size_t)len;

    param.message_id = (uint16_t)(sequence & 0xffffU);
    param.dup_flag = 0U;
    param.retain_flag = 0U;

    return mqtt_publish(&client, &param);
}

int main(void)
{
    uint32_t sequence = 1U;
    int rc;

    LOG_INF("Zephyr MQTT over STCP starting");
    LOG_INF("broker=%s:%d topic=%s",
            CONFIG_STCP_MQTT_BROKER_IPV4,
            CONFIG_STCP_MQTT_BROKER_PORT,
            CONFIG_STCP_MQTT_TOPIC);

    rc = broker_init();
    if (rc < 0) {
        LOG_ERR("broker_init failed: %d", rc);
        return rc;
    }

    while (true) {
        connected = false;
        transport.fd = -1;

        client_init();

        LOG_INF("Connecting MQTT using Zephyr MQTT library + STCP transport");

        rc = mqtt_connect(&client);
        if (rc < 0) {
            LOG_ERR("mqtt_connect failed: %d", rc);
            goto reconnect;
        }

        rc = service_until_connected(15000);
        if (rc < 0) {
            LOG_ERR("CONNACK wait failed: %d", rc);
            (void)mqtt_abort(&client);
            goto reconnect;
        }

        LOG_INF("MQTT connected over STCP");

        while (connected) {
            int64_t deadline;

            rc = publish_once(sequence++);
            if (rc < 0) {
                LOG_ERR("mqtt_publish failed: %d", rc);
                break;
            }

            LOG_INF("MQTT PUBLISH OK");

            deadline =
                k_uptime_get() + CONFIG_STCP_MQTT_PUBLISH_INTERVAL_MS;

            while (connected && k_uptime_get() < deadline) {
                rc = mqtt_service_once();

                if (rc < 0) {
                    LOG_ERR("MQTT service failed: %d", rc);
                    connected = false;
                    break;
                }

                k_sleep(K_MSEC(20));
            }
        }

        (void)mqtt_abort(&client);

reconnect:
        LOG_WRN("MQTT reconnect in %d ms",
                CONFIG_STCP_MQTT_RECONNECT_DELAY_MS);
        k_sleep(K_MSEC(CONFIG_STCP_MQTT_RECONNECT_DELAY_MS));
    }

    return 0;
}
