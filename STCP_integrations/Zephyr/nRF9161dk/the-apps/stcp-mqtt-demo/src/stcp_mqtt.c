#include <zephyr/random/random.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/init.h>
#include <modem/lte_lc.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/timeutil.h>
#include <zephyr/sys/heap_listener.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/kernel.h>


#include <stcp_api.h>
#include <stcp/debug.h>
#include <stcp/stcp_socket.h>
#include <stcp/utils.h>
#include <zephyr/net/mqtt.h>
#include <stcp/mqtt_stcp_stats.h>

#include "stcp_mqtt.h"

LOG_MODULE_REGISTER(stcp_mqtt_functions, LOG_LEVEL_INF);

#define SERVER_IP   "lja.fi"   // Linux STCP server
#define SERVER_PORT "7777"

struct mqtt_client client;
static struct sockaddr_storage broker;

int mqtt_connected = 0;

#define RX_BUF_SIZE 4096
#define TX_BUF_SIZE 4096
#define RECV_RETRY_DELAY_MS 5

static uint8_t rx_buffer[RX_BUF_SIZE];
static uint8_t tx_buffer[TX_BUF_SIZE];

int mqtt_server_stcp_recv_loop(struct stcp_api *api)
{
    int rc;

    rc = mqtt_input(&client);

    if (rc == -EAGAIN)
        return -EAGAIN;

    if (rc != 0) {
        LOG_ERR("mqtt_input failed rc=%d", rc);
        return rc;
    }

    rc = mqtt_live(&client);

    if (rc == -EAGAIN)
        return -EAGAIN;

    if (rc != 0) {
        LOG_ERR("mqtt_live failed rc=%d", rc);
        return rc;
    }

    return 0;
}

int mqtt_publish_message(const char *topic,
                         const uint8_t *payload,
                         size_t len)
{
    struct mqtt_publish_param param;

    param.message.topic.qos = MQTT_QOS_0_AT_MOST_ONCE;
    param.message.topic.topic.utf8 = (uint8_t *)topic;
    param.message.topic.topic.size = strlen(topic);

    param.message.payload.data = (uint8_t *)payload;
    param.message.payload.len  = len;

    param.message_id  = sys_rand32_get();
    param.dup_flag    = 0;
    param.retain_flag = 0;

    return mqtt_publish(&client, &param);
}

static void mqtt_evt_handler(struct mqtt_client *const client,
                             const struct mqtt_evt *evt)
{
    switch (evt->type) {

    case MQTT_EVT_CONNACK:
        LDBG("MQTT: CONNACK received, result=%d\n",
               evt->param.connack.return_code);

        if (evt->param.connack.return_code == MQTT_CONNECTION_ACCEPTED) {
            LDBG("MQTT: CONNECT OK\n");
            stpc_mqtt_subscribe(client);
            mqtt_connected = 1;
        } else {
            LDBG("MQTT: CONNECT FAILED\n");
            mqtt_connected = 0;
        }
        break;

    case MQTT_EVT_DISCONNECT:
        LDBG("MQTT: DISCONNECTED\n");
        mqtt_connected = 0;
        break;

    case MQTT_EVT_PUBLISH:
    {
        LDBG("MQTT: PUBLISH received\n");

        const struct mqtt_publish_param *p = &evt->param.publish;

        LDBG("MQTT: topic len %d\n", p->message.topic.topic.size);
        LDBG("MQTT: payload len %d\n", p->message.payload.len);

        uint8_t buf[256];
        int len = MIN(p->message.payload.len, sizeof(buf));

        mqtt_read_publish_payload(client, buf, len);

        printk("MQTT: payload: ");
        for (int i = 0; i < len; i++) {
            printk("%c", buf[i]);
        }
        printk("\n");

        break;
    }

    case MQTT_EVT_PUBACK:
        LDBG("MQTT: PUBACK received id=%d\n",
               evt->param.puback.message_id);
        break;

    case MQTT_EVT_SUBACK:
        LDBG("MQTT: SUBACK id=%d\n",
               evt->param.suback.message_id);
        break;

    case MQTT_EVT_PINGRESP:
        LDBG("MQTT: PINGRESP\n");
        break;

    default:
        LDBG("MQTT: event type %d\n", evt->type);
        break;
    }
}

int mqtt_connect_via_stcp(char *host, char *port, struct stcp_api **saveTo)
{
    LERRBIG("MQTT Connect via thread: %p", k_current_get());
    LDBGBIG("Starting to connect MQTT via STCP (%s:%s)", host, port);

    struct zsock_addrinfo *res = resolve_to_connect(host, port);

    if (!res) {
        LERRBIG("DNS resolve failed %s:%s (%d)", host, port, errno);
        return -EAGAIN;
    }

    stcp_util_log_sockaddr("MQTT connect resolved: ", res);

    memcpy(&broker, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    /* Init MQTT client */
    mqtt_client_init(&client);

    client.broker = &broker;
    client.evt_cb = mqtt_evt_handler;

    client.client_id.utf8 = "zephyr-stcp-device";
    client.client_id.size = strlen("zephyr-stcp-device");

    client.protocol_version = MQTT_VERSION_3_1_1;

    client.rx_buf = rx_buffer;
    client.rx_buf_size = sizeof(rx_buffer);

    client.tx_buf = tx_buffer;
    client.tx_buf_size = sizeof(tx_buffer);

    /* tärkeä */
    client.transport.type = MQTT_TRANSPORT_STCP;

    LDBG("MQTT connecting...");

    int rc = mqtt_connect(&client);

    if (rc < 0) {
        LDBG("MQTT connect failed rc=%d", rc);
        return rc;
    }

    LDBG("MQTT connect started");

    /* STCP API pointer löytyy vasta nyt */
    struct stcp_api *api = client.transport.stcp.stcp_api_instance;

    if (api) {
        int fd = stcp_api_get_fd(api);
        LDBG("STCP socket FD: %d", fd);

        if (saveTo) {
            *saveTo = api;
        }
    } else {
        LDBG("STCP API not available");
    }

    return 0;
}
