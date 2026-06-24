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
#include <zephyr/net/mqtt.h>
#include <fcntl.h>

#include <stcp_api.h>
#include <stcp/settings.h>
#include <stcp/utils.h>
#include <stcp/dns.h>
#include "stcp_mqtt.h"
#include <stcp/debug.h>

#include "mqtt_demo_utils.h"


#if CONFIG_MQTT_LIB_STCP

extern int mqtt_connected;
extern struct mqtt_client client;
K_MUTEX_DEFINE(client_lock);

int stpc_mqtt_subscribe(struct mqtt_client *clientPtr)
{
    static char *subTo = "testi";

    MDBG("Subscribing .... waiting for CONNACK event (max %d ms)",
        STCP_MQTT_WAIT_CONNACK_EVENT_FOR_MSEC);

    int ACK = stcp_mqtt_wait_for_connak_event(STCP_MQTT_WAIT_CONNACK_EVENT_FOR_MSEC);

    // 0 == Success
    if (ACK) {
        MWRN("Got no ACK event in %d ms...", STCP_MQTT_WAIT_CONNACK_EVENT_FOR_MSEC);
        return -EAGAIN;
    }

    if (!mqtt_connected) {
        MDBG("Not connected => NOT subbing...");
        return -EAGAIN;
    }

    MDBGBIG("Subscribing starts...");

    MDBG("Client API: %p", clientPtr->transport.stcp.stcp_api_instance);
    MDBG("Client FD: %d", clientPtr->transport.stcp.sock);

    MDBG("Subscribing to %s", subTo);

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

        
    CLIENT_LOCK(&client_lock);
    int ret = mqtt_subscribe(clientPtr, &subscription_list);
    CLIENT_UNLOCK(&client_lock);

    MDBG("mqtt_subscribe ret: %d", ret);
    stcp_mqtt_reset_connak_event_seen();
    return ret;

}


void make_timestamp(char *out, size_t max_len)
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
#endif // CONFIG_MQTT_LIB_STCP

int main(void)
{

#if CONFIG_MQTT_LIB_STCP
//#  if STCP_COMPILE_MQTT

    int rc = stcp_library_init();
    if (rc != 0) {
        MWRN("STCP init error, rc: %d", rc);
    } else {
        MINF("STCP READY!");
    }

    MDBG("Doign DNS resolving");
    stcp_dns_resolve_all();

# if (CONFIG_STCP_TESTING && (CONFIG_STCP_TEST_MODE == 3))
    MINF("FSM starting....");
    stpc_mqtt_init_fsm_thread();
    MINF("FSM started....");
# else
#  if !CONFIG_STCP_TESTING
    MINF("FSM starting....");
    stpc_mqtt_init_fsm_thread();
    MINF("FSM started....");
#  endif
# endif

    while (1) {
        SLEEP_SEC(600);
    }

    return 0;
#endif // CONFIG_MQTT_LIB_STCP
}
