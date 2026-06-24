#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>
#include <modem/lte_lc.h>
#include <zephyr/logging/log.h>

#include <stcp/stcp_api.h>
#include "stcp_mqtt_fsm.h"

#define STCP_MQTT_LOCKING_VERBOSE           0
#define RX_BUF_SIZE                         512
#define TX_BUF_SIZE                         512

struct mqtt_ctx {
    struct k_mutex lock;
    void *client_ptr;
    void *broker_ptr;
    enum mqtt_state state;
    enum mqtt_state last_state;
    uint32_t last_progress;
    uint32_t dead_counter;
    uint32_t eagain_counter;
    uint32_t loop_count;
    int stop;
    uint8_t rx_buffer[RX_BUF_SIZE];
    uint8_t tx_buffer[TX_BUF_SIZE];
};

#define MQTT_API_CLIENT_DEBUG(clientVoid)   \
    do {                                                \
        struct mqtt_client *client = clientVoid;        \
        struct stcp_api *api =                          \
            API_FROM_CLIENT(client);                    \
        if (client) {                                   \
            LERRBIG(                                    \
                "CLIENT=%p API=%p SOCK=%d STATE=%d",    \
                client,                                 \
                api,                                    \
                client->transport.stcp.sock,            \
                mqtt_ctx->state                         \
            );                                          \
        }                                               \
    } while(0)

int stpc_mqtt_subscribe(struct mqtt_client *client);

int mqtt_connect_via_stcp(char *host, int port, struct mqtt_ctx *mqtt_ctx, struct stcp_api **saveTo);

int mqtt_server_stcp_recv_loop(struct stcp_api *api);

int mqtt_publish_message(struct mqtt_ctx *mqtt_ctx,
                         const char *topic,
                         const uint8_t *payload,
                         size_t len);

int stcp_mqtt_reconnect(struct mqtt_ctx *mqtt_ctx);
int stcp_mqtt_run_session(struct stcp_api *api);
void make_timestamp(char *out, size_t max_len);

#define MQTT_CONTEXT_LOCK_TIMEOUT_SECONDS      120


#define GET_MQTT_CTX_FROM(val)      ((struct mqtt_ctx*)(val))

#define GET_MQTT_CTX_LOCK(val)      (GET_MQTT_CTX_FROM(val)->lock)

#if STCP_MQTT_LOCKING_VERBOSE

#define MQTT_CTX_LOCK(clientArg) \
    do {                                                      \
        MDBG("MQTT: Locking %p client...timeout %d seconds",  \
            clientArg, MQTT_CONTEXT_LOCK_TIMEOUT_SECONDS);    \
        int __to = k_mutex_lock(                              \
            & GET_MQTT_CTX_LOCK(clientArg),                   \
            K_SECONDS(MQTT_CONTEXT_LOCK_TIMEOUT_SECONDS));    \
        MDBG("MQTT: Locked %p client... RC: %d",              \
            clientArg, __to);                                 \
    } while (0)

#define MQTT_CTX_UNLOCK(clientArg) \
    do {                                                  \
        MDBG("MQTT: Unlocking %p client...", clientArg);  \
        k_mutex_unlock(&GET_MQTT_CTX_LOCK(clientArg));    \
        MDBG("MQTT: Unlocked %p client...", clientArg);   \
    } while (0)

#else // no STCP_MQTT_LOCKING_VERBOSE

#define MQTT_CTX_LOCK(clientArg) \
        k_mutex_lock(                              \
            & GET_MQTT_CTX_LOCK(clientArg),                   \
            K_SECONDS(MQTT_CONTEXT_LOCK_TIMEOUT_SECONDS));    \

#define MQTT_CTX_UNLOCK(clientArg) \
        k_mutex_unlock(&GET_MQTT_CTX_LOCK(clientArg));    \


#endif