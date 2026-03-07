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
#include <zephyr/net/mqtt.h>
#include <fcntl.h>

#include <stcp_api.h>
#include <stcp/settings.h>
#include <stcp/utils.h>
#include "stcp_mqtt.h"

LOG_MODULE_REGISTER(stcp_mqtt_demo, LOG_LEVEL_INF);


extern int mqtt_connected;
extern struct mqtt_client client;

int stpc_mqtt_subscribe(struct mqtt_client *client)
{
    static char *subTo = "testi";

    LDBG("Client API: %p", client->transport.stcp.stcp_api_instance);
    LDBG("Client FD: %d", client->transport.tcp.sock);

    LDBG("Subscribing to %s", subTo);
    struct mqtt_topic subscribe_topic = {
        .topic = {
            .utf8 = subTo,
            .size = strlen(subTo),
        },
        .qos = MQTT_QOS_0_AT_MOST_ONCE
    };

    const struct mqtt_subscription_list subscription_list = {
        .list = &subscribe_topic,
        .list_count = 1,
        .message_id = 1
    };
    LDBG("Subbing ....");
    return mqtt_subscribe(client, &subscription_list);
}


static void make_timestamp(char *out, size_t max_len)
{
    int64_t now_ms = k_uptime_get();

    time_t now_sec = now_ms / 1000;

    struct tm tm;
    gmtime_r(&now_sec, &tm);

    snprintf(out, max_len,
             "STCP DEBUG SENT AT %04d-%02d-%02dT%02d:%02d:%02dZ",
             tm.tm_year + 1900,
             tm.tm_mon + 1,
             tm.tm_mday,
             tm.tm_hour,
             tm.tm_min,
             tm.tm_sec);
}

int main(void)
{
    struct stcp_api *theAPI = NULL;

    int rc = stcp_library_init();
    LINF("STCP library init: %d, errno: %d .. waiting it to complete..\n", rc, errno);

    // minute waiting max ..
    rc = mqtt_connect_via_stcp("lja.fi", "7777", &theAPI);

    LINF("STCP + MQTT READY!");

    while (1) {

        int r = mqtt_server_stcp_recv_loop(theAPI);

        if (r < 0) {
            if (r == -EAGAIN) {
                k_sleep(K_MSEC(5));
                continue;
            }
            LOG_ERR("STCP recv loop failed rc=%d", r);
            break;
        }

        if (mqtt_connected) {

            char timestamp[128];
            make_timestamp(timestamp, sizeof(timestamp));

            mqtt_publish_message(
                "testi",
                (uint8_t *)timestamp,
                strlen(timestamp)
            );
        }

        k_sleep(K_MSEC(100));
    }

    return 0;
}
