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

#include <stcp/stcp_api.h>
#include <stcp/debug.h>
#include <stcp/stcp_socket.h>
#include <stcp/utils.h>
#include <zephyr/net/mqtt.h>
#include <stcp/mqtt_stcp_stats.h>
#include <stcp/dns.h>

#include "stcp_mqtt.h"
#include "stcp_mqtt_fsm.h"
#include "mqtt_demo_utils.h"

// TODO: Tämäkin globaali pois?
extern struct k_mutex client_lock;
int mqtt_connected = 0;

#define RECV_RETRY_DELAY_MS                 5

// 4min
#define STCP_CONNECTION_WAIT_TIMEOUT_MS     (60*1000)

#if 0
int stcp_mqtt_reconnect(struct mqtt_ctx *mqtt_ctx)
{
    int rc;

    if (!mqtt_ctx) {
        return -EBADMSG;        
    }

    MQTT_CTX_LOCK(mqtt_ctx);

        struct mqtt_client *client = mqtt_ctx->client_ptr;

        if (!client) {
            MQTT_CTX_UNLOCK(mqtt_ctx);
            return -EBADMSG;        
        }

        struct stcp_api* api = client->transport.stcp.stcp_api_instance;
        LINF("MQTT reconnect attempt api=%p fd=%d conn=%d hs=%d",
            api,
            stcp_api_get_fd(api),
            api->connected,
            stcp_api_get_handshake_status(api)
        );


        mqtt_abort(client);
        LINF("MQTT reconnect: Abort");

        SLEEP_MSEC(25);

        rc = mqtt_disconnect(client, NULL);
        LINF("MQTT reconnect: Disconnect %p, %d",
            client->transport.stcp.stcp_api_instance,
            rc
        );

        SLEEP_MSEC(1000);

        LINF("MQTT reconnect: Disconnect wait done for %p, %d",
            client->transport.stcp.stcp_api_instance,
            rc
        );

        LDBG("MQTT: FULL init of client struct @ reconnect....");
        mqtt_client_prepare(mqtt_ctx, 1);

        LERR(
            "MQTT: Reconnect: CLIENT API AFTER PREPARE: %p",
            API_FROM_CLIENT(client)
        );

        LINF("MQTT: Connect of client via %d", client->transport.stcp.sock);
        LERR(
            "MQTT: Reconnect: BEFORE CONNECT client=%p api=%p",
            client,
            API_FROM_CLIENT(client)
        );
        rc = mqtt_connect(client);

        LERR(
            "MQTT: Reconnect: AFTER CONNECT client=%p api=%p",
            client,
            API_FROM_CLIENT(client)
        );

        // API Saatavilla nyt!
        api = API_FROM_CLIENT(client);
        if (api == NULL) {
            LDBG("MQTT: Creating API failed!");
            MQTT_CTX_UNLOCK(mqtt_ctx);
            return rc;
        }

        // Sanity check 
        rc = stcp_api_is_alive(api);
        LINF("MQTT reconnect: refsh API Handle: %p, alive? %d", api, GET_YES_NO_STR(rc > 0));

        if (rc != 1) {
            LERR("MQTT reconnect failed, not alive sock... rc=%d", rc);
            MQTT_CTX_UNLOCK(mqtt_ctx);
            return rc;
        }

        LDBG("MQTT: Got new MQTT API is %p with fd %d", 
            api,
            stcp_api_get_fd(api)
        );

        LINF("MQTT: Waiting until connected ..... %d seconds max..",
                STCP_CONNECTION_WAIT_TIMEOUT_MS);
        rc = stcp_api_wait_until_connected_to_peer_no_lock(api, STCP_CONNECTION_WAIT_TIMEOUT_MS);
        LDBG("MQTT: Connection waited, rc: %d", rc);

        LINF("MQTT reconnect OK");

        SLEEP_MSEC(25);

        /* resubscribe */
        rc = stpc_mqtt_subscribe(client);
        if (rc < 0) {
            MWRN("Subscribe failed: %d", rc);
        }

    MQTT_CTX_UNLOCK(mqtt_ctx);

    return rc;
}
#endif 

int mqtt_publish_message(struct mqtt_ctx *mqtt_ctx,
                        const char *topic,
                         const uint8_t *payload,
                         size_t len)
{
    int ret = -EAGAIN;
    LDBGBIG("Publish called with topic %s", topic);

    if (!mqtt_ctx || !mqtt_ctx->client_ptr) {
        LDBG("MQTT: Publish, got no valid client!");
        return ret;
    }


    struct mqtt_client *client = mqtt_ctx->client_ptr;
    struct stcp_api *api = API_FROM_CLIENT(client);

    if (!api) {
        LERR("MQTT: Publish message: No API...");
        goto out;
    }

    if (!api->connack_seen) {
        LERR("MQTT: Publish message: NoCONNACK seen ...");
        goto out;
    }

    if (! atomic_get(&api->connected) ) {
        LERR("MQTT: Publish message: Not connected...");
        goto out;
    }

    LINF("MQTT: Publish message:");
    stcp_hexdump_ascii(topic, payload, len);

    struct mqtt_publish_param param;

    param.message.topic.qos = MQTT_QOS_1_AT_LEAST_ONCE;
    param.message.topic.topic.utf8 = (uint8_t *)topic;
    param.message.topic.topic.size = strlen(topic);

    param.message.payload.data = (uint8_t *)payload;
    param.message.payload.len  = len;

    param.message_id  = sys_rand32_get();
    param.dup_flag    = 0;
    param.retain_flag = 0;

    LDBGBIG("Publish via %d with topic %s", client->transport.stcp.sock, topic);
    
    ret = mqtt_publish(client, &param);
    LDBG("MQTT: Publish returned %d", ret);
out:
    return ret;
}

static void mqtt_evt_handler(struct mqtt_client *const clientPtr,
                             const struct mqtt_evt *evt)
{

    LINF("MQTT EVT type=%d", evt->type);

    struct stcp_api *api = API_FROM_CLIENT(clientPtr);
    int aquired = stcp_api_acquire(api);

    LINF("MQTT: Client %p => API %p",
        clientPtr, api
    );

    if (evt->type == MQTT_EVT_CONNACK) {
        LINF("MQTT EVT CONNACK rc=%d",  evt->param.connack.return_code);
    }

    switch (evt->type) {

        case MQTT_EVT_CONNACK: {
            LDBG("MQTT: CONNACK received, result=%d\n",
                evt->param.connack.return_code);

            LDBGBIG("MQTT: Got api at event handler: %p", api);
            if (evt->param.connack.return_code == MQTT_CONNECTION_ACCEPTED) {
                LDBG("MQTT: CONNACK OK");
                stcp_mqtt_set_connak_event_seen(api);
                LDBG("MQTT: CONNACK => API %p updated!", api);
            } else {
                LDBG("MQTT: CONNACK RESET");
                stcp_mqtt_reset_connak_event_seen(api);
            }

            break;
        }

        case MQTT_EVT_DISCONNECT: {
            LDBG("MQTT: DISCONNECTED\n");
            struct stcp_api *api = API_FROM_CLIENT(clientPtr);
            stcp_mqtt_reset_connak_event_seen(api);
            break;
        }

        case MQTT_EVT_PUBLISH: {
            struct stcp_api *api = API_FROM_CLIENT(clientPtr);
            LDBG("MQTT: PUBLISH received (API: %p)\n", api);

            const struct mqtt_publish_param *p = &evt->param.publish;

            LDBG("MQTT: topic len %d\n", p->message.topic.topic.size);
            LDBG("MQTT: payload len %d\n", p->message.payload.len);
#define PUBLISH_BUFFER_SIZE 256
            uint8_t buf[PUBLISH_BUFFER_SIZE];
            int len = MIN(p->message.payload.len, PUBLISH_BUFFER_SIZE);

            mqtt_read_publish_payload(clientPtr, buf, len);

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

    if(aquired) {
        stcp_api_release(api);
    }
}

static struct zsock_addrinfo *mqtt_dns_info = NULL;

int mqtt_dns_info_free() 
{
    return stcp_dns_free(mqtt_dns_info);
}

void mqtt_client_prepare(struct mqtt_ctx *mqtt_ctx, int do_memset)
{
    if (!mqtt_ctx) {
        LWRN("MQTT: Prepare: No MQTT context!");
        return;
    }

    MQTT_CTX_LOCK(mqtt_ctx);
        struct mqtt_client *client = mqtt_ctx->client_ptr;

        if (!client) {
            LERR("MQTT: Prepare: No client");
            return;
        }

        if (!mqtt_ctx->broker_ptr) {
            LERR("MQTT: Prepare: No broker");
            return;
        }

        if (do_memset) {
            LWRN("MQTT: Prepare: DOING MEMSET!");

            memset(
                mqtt_ctx->rx_buffer,
                0,
                sizeof(mqtt_ctx->rx_buffer)
            );

            memset(
                mqtt_ctx->tx_buffer,
                0,
                sizeof(mqtt_ctx->tx_buffer)
            );
            
            memset(mqtt_ctx->client_ptr, 0, sizeof(*mqtt_ctx->client_ptr));
            memset(mqtt_ctx->broker_ptr, 0, sizeof(*mqtt_ctx->broker_ptr));

        }


        mqtt_client_init(client);

        client->broker = mqtt_ctx->broker_ptr;
        client->client_id.utf8 = "zephyr-stcp-device";
        client->client_id.size = strlen("zephyr-stcp-device");
        client->keepalive = STCP_MQTT_KEEPALIVE_SECONDS;
        client->protocol_version = MQTT_VERSION_3_1_1;

        client->rx_buf = mqtt_ctx->rx_buffer;
        client->rx_buf_size = sizeof(mqtt_ctx->rx_buffer);
        client->tx_buf = mqtt_ctx->tx_buffer;
        client->tx_buf_size = sizeof(mqtt_ctx->tx_buffer);

        /* tärkeä */
        client->transport.type = MQTT_TRANSPORT_STCP;

        client->evt_cb = mqtt_evt_handler;   // 🔥 PAKOLLINEN

    MQTT_CTX_UNLOCK(mqtt_ctx);
}

enum mqtt_connection_state mqtt_stcp_connection_status_get(struct mqtt_ctx *mqtt_ctx) {
    enum mqtt_connection_state ret = MQTT_CONNECTION_STATE_DISCONNECTED;

    if (!mqtt_ctx) { return ret; }
    if (!mqtt_ctx->client_ptr) { return ret; }

    struct stcp_api *api = API_FROM_CLIENT(mqtt_ctx->client_ptr);
    if (!api) { return ret; }

    if (api && stcp_api_is_alive(api))
    {
        int connect_in_progress = stcp_api_get_connect_in_progress(api);
        int connected = stcp_api_get_handshake_status(api);
        LINF("MQTT: Connect in progress: %s, Connected: %s",
            GET_YES_NO_STR(connect_in_progress),
            GET_YES_NO_STR(connected)
        );

        if (connected) {
            ret = MQTT_CONNECTION_STATE_CONNECT;
        } else if (connect_in_progress > 0) {
            ret = MQTT_CONNECTION_STATE_CONNECTING;
        }
    }

    return ret;
}

enum mqtt_connection_state mqtt_connection_status_get(struct mqtt_ctx *mqtt_ctx) {
    enum mqtt_connection_state stcp_state = mqtt_stcp_connection_status_get(mqtt_ctx);
    enum mqtt_connection_state ret = MQTT_CONNECTION_STATE_DISCONNECTED;

    if (!mqtt_ctx)              { return ret; }
    if (!mqtt_ctx->client_ptr)  { return ret; }

    struct stcp_api *api = API_FROM_CLIENT(mqtt_ctx->client_ptr);
    if (!api)                   { return ret; }
 


    switch(ret) {
        case MQTT_CONNECTION_STATE_CONNECT:
            LDBG("MQTT: STCP Connected");
            break;
        case MQTT_CONNECTION_STATE_CONNECTING:
            LDBG("MQTT: STCP Connecting");
            break;
        default:
            LDBG("MQTT: STCP Disconnected");
            return ret;
            break;
    }

    return ret;
}


int mqtt_connect_via_stcp(char *host, int port, struct mqtt_ctx *mqtt_ctx, struct stcp_api **saveTo)
{
    struct stcp_api *api = NULL;
    struct mqtt_client *client = NULL;
    struct sockaddr_storage *broker = NULL;

    if (!mqtt_ctx) {
        LERR("MQTT: Connect: No MQTT CTX");
        return -EBADMSG;
    } else {
        // Nyt pointterit ...
        client = mqtt_ctx->client_ptr;
        broker = mqtt_ctx->broker_ptr;

    }
    if (!client) {
        LERR("NO Client!");
        return -EBADMSG;
    }
    
    if (!broker) {
        LERR("NO Broker!");
        return -EBADMSG;
    }
    

    LERRBIG("MQTT Connect: via thread: %p", k_current_get());
    LDBGBIG("MQTT Connect: Starting to connect MQTT via STCP (%s:%d)", host, port);
    int rc = -1;

    //LERR("MQTT Connect: Backtrace of the callee....");
    //stcp_dump_bt();
    api = API_FROM_CLIENT(client);
    if (api && stcp_api_is_alive(api))
    {
        int connect_in_progress = stcp_api_get_connect_in_progress(api);
        LWRN("MQTT: Connect: Got already live API.. Connect in progress: %s",
            GET_YES_NO_STR(connect_in_progress)
        );

        if (connect_in_progress > 0) {
            LWRN("MQTT: Connect already in progress, skipping reconnect");
            return -EALREADY;
        }

    }

    // Ensin tämä
    mqtt_client_prepare(mqtt_ctx, 1);
    // Nyt pointterit ...
    client = mqtt_ctx->client_ptr;
    broker = mqtt_ctx->broker_ptr;


    if (mqtt_dns_info == NULL) {
        rc = stcp_dns_resolve(host, port, &mqtt_dns_info);
    }

    MQTT_CTX_LOCK(mqtt_ctx);

        if (!mqtt_dns_info) {
            LERRBIG("MQTT Connect: DNS resolve failed %s:%d (%d)", host, port, errno);
            MQTT_CTX_UNLOCK(mqtt_ctx);
            return -EAGAIN;
        }

        stcp_util_log_sockaddr("MQTT Connect: connect resolved: ", mqtt_dns_info);

        LERR(
            "MQTT: STCP Connect: CLIENT API AFTER PREPARE: %p",
            API_FROM_CLIENT(client)
        );

        if (!client) {
            LERR("MQTT: Connect: No MQTT client");
            MQTT_CTX_UNLOCK(mqtt_ctx);
            return -EBADMSG;
        }

        if (!broker) {
            LERR("MQTT: Connect: No MQTT broker");
            MQTT_CTX_UNLOCK(mqtt_ctx);
            return -EBADMSG;
        }

        memcpy(broker, mqtt_dns_info->ai_addr, mqtt_dns_info->ai_addrlen);
        LDBG("MQTT: FULL init of client struct @ connect....");

    MQTT_CTX_UNLOCK(mqtt_ctx);

    if (client) {
        LERR(
            "MQTT: STCP Connect: BEFORE CONNECT client=%p api=%p",
            client,
            API_FROM_CLIENT(client)
        );

        rc = mqtt_connect(client);
        LDBGBIG(
            "MQTT: MQTT CONNECT RET=%d errno=%d api=%p",
            rc,
            errno,
            API_FROM_CLIENT(client)
        );
        
        api = API_FROM_CLIENT(client);
    }

    if (!api) {
        LDBG("STCP API not available");
        return -ENODEV;
    }

    LINF("MQTT Connect: Keepalive: %d (set: %d)", STCP_MQTT_KEEPALIVE_SECONDS, client->keepalive);

    // OK, handletaan ....

    int tries_left = 5*60;
    int wait_for = 1*1000;
    LDBGBIG("MQTT: Connection wait until connected, Tries: %d, wait %d seconds",
        tries_left, wait_for);
    int connection_ok = 0;
    while ((tries_left > 0) && (!connection_ok)) {
        int radio = 0;

        stcp_api_get_modem_state(NULL, NULL, NULL, &radio, &connection_ok);

        if (!radio) {
            LDBGBIG("Radio OFF, trying to wake it up!");
            stcp_api_try_to_wakeup_radio();
        }

        int wait_rc = stcp_api_wait_until_connected_to_peer(api, wait_for);
        LDBGBIG("MQTT: Connection wait: Modem state[radio enabled: %s,  connected ok: %s], Triesleft: %d, wait %d seconds",
            GET_YES_NO_STR(radio),
            GET_YES_NO_STR(connection_ok),
            tries_left, wait_for);

        if (connection_ok) {
            LDBG("Wait completed!");
            break;
        }

        tries_left--; // Vähennetään ENNEN nii on looginen
        
        if (wait_rc < 0) {
            if (tries_left == 0) {
                LDBG("Wait error: %d", wait_rc);
                if (api != NULL) {

                    STCP_REF_COUNT_PUT(
                        api->ctx,
                        "the MQTT reference"
                    );

                    LDBG("Closing API...");
                    stcp_api_close(api);
                } 
                LERR("Returnign wait_rc: %d", wait_rc);
                return wait_rc;
            }
        }
    }

    LERR(
        "MQTT Info socket=%d transport.type=%d api=%p connected=%d handshake=%d alive=%d",
        client->transport.stcp.sock,
        client->transport.type,
        api,
        api ? atomic_get(&api->ctx->connected) : -1,
        api ? api->ctx->handshake_done : -1,
        api ? atomic_get(&api->alive) : -1
    );

    if (api && api->ctx) {
        LDBG("Context available for dump...");
        stcp_debug_dump_stcp_ctx(api->ctx);
    } else {
        LDBG("NO context available for dump...");
    }

    if (saveTo) {
        *saveTo = api;
    }

    LDBGBIG("MQTT Connect: OK");
    return 0;
}
