#include <zephyr/random/random.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
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
#include <zephyr/posix/fcntl.h>

#include <stcp_api.h>
#include <stcp/debug.h>
#include <stcp/stcp_socket.h>
#include <stcp/utils.h>
#include <zephyr/net/mqtt.h>
#include <stcp/mqtt_stcp_stats.h>
#include "mqtt_demo_utils.h"

#include "stcp_mqtt.h"


#define SERVER_IP   "lja.fi"   // Linux STCP server
#define SERVER_PORT "7777"

struct mqtt_client client;
static struct sockaddr_storage broker;
extern struct k_mutex client_lock;
int mqtt_connected = 0;

#define RX_BUF_SIZE             1024
#define TX_BUF_SIZE             1024
#define RECV_RETRY_DELAY_MS     5


static uint8_t rx_buffer[RX_BUF_SIZE];
static uint8_t tx_buffer[TX_BUF_SIZE];

int stcp_mqtt_reconnect(struct mqtt_client *client)
{
    int rc;

    MINF("MQTT reconnect attempt");

    CLIENT_LOCK(&client_lock);
    mqtt_disconnect(client);
    CLIENT_UNLOCK(&client_lock);

    SLEEP_SEC(1);

    CLIENT_LOCK(&client_lock);
    rc = mqtt_connect(client);
    CLIENT_UNLOCK(&client_lock);

    if (rc != 0) {
        LERR("MQTT reconnect failed rc=%d", rc);
        return rc;
    }

    MINF("MQTT reconnect OK");

    /* resubscribe */
    rc = stpc_mqtt_subscribe(client);
    if (rc) {
        MWRN("Subscribe failed: %d", rc);
    }

    return rc;
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

    CLIENT_LOCK(&client_lock);
        int ret = mqtt_publish(&client, &param);
    CLIENT_UNLOCK(&client_lock);
    return ret;
}

static void mqtt_evt_handler(struct mqtt_client *const clientPtr,
                             const struct mqtt_evt *evt)
{
    switch (evt->type) {

        case MQTT_EVT_CONNACK:
            MDBG("MQTT: CONNACK received, result=%d\n",
                evt->param.connack.return_code);

            if (evt->param.connack.return_code == MQTT_CONNECTION_ACCEPTED) {
                MDBG("MQTT: CONNECT OK\n");
                mqtt_connected = 1;
                stcp_mqtt_set_connak_event_seen();
            } else {
                MDBG("MQTT: CONNECT FAILED\n");
                stcp_mqtt_reset_connak_event_seen();
                mqtt_connected = 0;
            }


            break;

        case MQTT_EVT_DISCONNECT:
            MDBG("MQTT: DISCONNECTED\n");
            mqtt_connected = 0;
            stcp_mqtt_reset_connak_event_seen();
            break;

        case MQTT_EVT_PUBLISH:
        {
            MDBG("MQTT: PUBLISH received\n");
            mqtt_connected = 1;

            const struct mqtt_publish_param *p = &evt->param.publish;

            MDBG("MQTT: topic len %d\n", p->message.topic.topic.size);
            MDBG("MQTT: payload len %d\n", p->message.payload.len);

            uint8_t buf[256];
            int len = MIN(p->message.payload.len, sizeof(buf));

            CLIENT_LOCK(&client_lock);
            mqtt_read_publish_payload(clientPtr, buf, len);
            CLIENT_UNLOCK(&client_lock);

            printk("MQTT: payload: ");
            for (int i = 0; i < len; i++) {
                printk("%c", buf[i]);
            }
            printk("\n");

            break;
        }

        case MQTT_EVT_PUBACK:
            MDBG("MQTT: PUBACK received id=%d\n",
                evt->param.puback.message_id);
            break;

        case MQTT_EVT_SUBACK:
            MDBG("MQTT: SUBACK id=%d\n",
                evt->param.suback.message_id);
            break;

        case MQTT_EVT_PINGRESP:
            MDBG("MQTT: PINGRESP\n");
            break;

        default:
            MDBG("MQTT: event type %d\n", evt->type);
            break;
        }
}

int mqtt_connect_via_stcp(char *host, char *port, struct stcp_api **saveTo)
{
    LERRBIG("MQTT Connect via thread: %p", k_current_get());
    LDBGBIG("Starting to connect MQTT via STCP (%s:%s)", host, port);
    CLIENT_LOCK(&client_lock);

    struct zsock_addrinfo *res = resolve_to_connect(host, port);

    if (!res) {
        LERRBIG("DNS resolve failed %s:%s (%d)", host, port, errno);
        CLIENT_UNLOCK(&client_lock);
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

    client.keepalive = STCP_MQTT_KEEPALIVE_SECONDS;
    client.protocol_version = MQTT_VERSION_3_1_1;

    client.rx_buf = rx_buffer;
    client.rx_buf_size = sizeof(rx_buffer);

    client.tx_buf = tx_buffer;
    client.tx_buf_size = sizeof(tx_buffer);

    /* tärkeä */
    client.transport.type = MQTT_TRANSPORT_STCP;


    MDBG("MQTT connecting...");

    int rc = mqtt_connect(&client);

    LINF("Client info:");
    LINF("  Keepalive: %d (set: %d)", STCP_MQTT_KEEPALIVE_SECONDS, client.keepalive);

    if (rc < 0) {
       MDBG("MQTT connect failed rc=%d, errno: %d", rc, errno);
       CLIENT_UNLOCK(&client_lock);
       return errno;
    }

    MDBG("MQTT connect started");

    /* STCP API pointer löytyy vasta nyt */
    struct stcp_api *api = client.transport.stcp.stcp_api_instance;

    if (api) {
        int fd = stcp_api_get_fd(api);
        int flags = zsock_fcntl(fd, F_GETFL, 0);
        zsock_fcntl(fd, F_SETFL, flags | O_NONBLOCK);

        MDBG("STCP socket (set not to block) FD: %d", fd);
        if (saveTo) {
            *saveTo = api;
        }
    } else {
        MDBG("STCP API not available");
    }
    CLIENT_UNLOCK(&client_lock);

    return 0;
}
