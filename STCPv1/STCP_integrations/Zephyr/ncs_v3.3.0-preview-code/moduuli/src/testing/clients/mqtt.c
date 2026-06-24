
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>

#include <stdbool.h>
#include <errno.h>
#include <stdint.h>
#include <time.h>
#include "testing/clients/stcp_testing_common.h"

#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/debug.h>

#define LOGTAG     "[STCP/Torture/MQTT] "
#include <stcp_testing_bplate.h>

#include <stcp_testing_common.h>



struct mqtt_client client;
static struct sockaddr_storage broker;
struct k_mutex client_lock;
int mqtt_connected = 0;

void mqtt_torture_worker(void *p1, void *p2, void *p3)
{
    struct worker_ctx *ctx = p1;

    uint8_t payload[256];

    struct mqtt_client client;

    /* MQTT init täällä */
 
    while (1) {

        while (mqtt_connected) {
            int len = generate_strftime_payload(payload, sizeof(payload));
            struct mqtt_publish_param param;

            memset(&param, 0, sizeof(param));

            param.message.topic.qos = MQTT_QOS_1_AT_LEAST_ONCE;
            param.message.topic.topic.utf8 = "stcp/test";
            param.message.topic.topic.size = strlen("stcp/test");

            param.message.payload.data = payload;
            param.message.payload.len = len;

            mqtt_publish(&client, &param);
        }
    }
}

//
// MQTT funkkarit
//

#define CLIENT_LOCK(clientArg) \
    do {                                       \
        MDBG("MQTT: Locking %p client...", clientArg);    \
        k_mutex_lock((clientArg), K_FOREVER); \
        MDBG("MQTT: Locked %p client...", clientArg);     \
    } while (0)

#define CLIENT_UNLOCK(clientArg) \
    do {                                       \
        MDBG("MQTT: Unlocking %p client...", clientArg);  \
        k_mutex_unlock((clientArg));          \
        MDBG("MQTT: Unlocked %p client...", clientArg);   \
    } while (0)

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


int mqtt_connect_via_stcp(struct stcp_api **saveTo)
{
    LERRBIG("MQTT Connect via thread: %p", k_current_get());
    LDBGBIG("Starting to connect MQTT via STCP (%s:%s)", host, port);
    CLIENT_LOCK(&client_lock);

    stcp_util_log_sockaddr("MQTT connect resolved: ", &aeragerg);

    memcpy(&broker, 
        the_test_server_addr_resolved.ai_addr, 
        the_test_server_addr_resolved.ai_addrlen);

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
