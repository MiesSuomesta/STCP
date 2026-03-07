#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>
#include <modem/lte_lc.h>
#include <zephyr/logging/log.h>

#include <stcp_api.h>
#include "stcp_mqtt.h"

LOG_MODULE_REGISTER(stcp_mqtt_demo, LOG_LEVEL_INF);

#define SERVER_IP   "lja.fi"   // Linux STCP server
#define SERVER_PORT 7777

static struct mqtt_client client;
static struct sockaddr_storage broker;

// 4kb send/recv buffers
static uint8_t rx_buffer[1024*4];
static uint8_t tx_buffer[1024*4];

static void mqtt_evt_handler(struct mqtt_client *c,
                              const struct mqtt_evt *evt)
{
    switch (evt->type) {
    case MQTT_EVT_CONNACK:
        LOG_INF("MQTT connected");
        break;

    case MQTT_EVT_PUBLISH:
        LOG_INF("MQTT publish received");
        break;

    default:
        break;
    }
}

int mqtt_connect_via_stcp(int fd)
{
    mqtt_client_init(&client);

    client.broker = &broker;
    client.evt_cb = mqtt_evt_handler;
    client.client_id.utf8 = "zephyr-device";
    client.client_id.size = strlen("zephyr-device");

    client.password = NULL;
    client.user_name = NULL;

    client.protocol_version = MQTT_VERSION_3_1_1;

    client.rx_buf = rx_buffer;
    client.rx_buf_size = sizeof(rx_buffer);
    client.tx_buf = tx_buffer;
    client.tx_buf_size = sizeof(tx_buffer);

    client.transport.type = MQTT_TRANSPORT_NON_SECURE;
    client.transport.tcp.sock = fd;  // 👈 STCP socket

    return mqtt_connect(&client);
}

