#pragma once
#include <zephyr/random/random.h>
#include <zephyr/kernel.h>
#include <zephyr/sys_clock.h>

#include <stcp/debug.h>
#include "stcp_mqtt.h"

#define STCP_MQTT_WAIT_CONNACK_EVENT_FOR_MSEC    (4 * 60 * 1000)
#define STCP_MQTT_POLL_TIMEOUT_MSEC              (1 * 1000)

// IRS ystävällinen
static inline uint32_t mqtt_get_timestamp(void)
{
    return (uint32_t)k_cyc_to_ms_floor32(k_cycle_get_32());
}

#define VOID_TO_MQTT_CLIENT(ptr) ((struct mqtt_client *)(ptr))

#define API_FROM_CLIENT(clientPtr) ((clientPtr) ? VOID_TO_MQTT_CLIENT(clientPtr)->transport.stcp.stcp_api_instance : NULL)
#define API_SET_TO_CLIENT(clientPtr, val) \
    do {                                                                                                        \
        if ((clientPtr) != NULL) { \
            VOID_TO_MQTT_CLIENT(clientPtr)->transport.stcp.stcp_api_instance = val;     \
            VOID_TO_MQTT_CLIENT(clientPtr)->transport.stcp.sock = stcp_api_get_fd(val); \
            LDBGBIG("MQTT CLIENT %p API %p set as %p ...",                              \
                VOID_TO_MQTT_CLIENT(clientPtr),                                         \
                VOID_TO_MQTT_CLIENT(clientPtr)->transport.stcp.stcp_api_instance,       \
                val);                                                                   \
        }                                                                               \
    } while (0)


#define MQTT_FSM_STATE_CHECK(fsm, val) \
        fms_state_change_validation(fsm, val)

#define MQTT_FSM_STATE_SET(fsm, val)               \
    do {                                                \
        if (STCP_STCP_FSM_STATE_CHECK(fsm, val) == 1) {  \
            VOID_TO_STCP_FSM(fsm)->state = val;         \
            STCP_STCP_FSM_STATE_TRACE(fsm, val);        \
        }                                               \
    } while(0)



// Tuotantoon 30 - 300 sec, mut MAX KEEPALIVE sec
#if CONFIG_STCP_TESTING && (CONFIG_STCP_TESTING_MODE == 3)
#define STCP_MQTT_PUBLISH_INTERVAL_MS            (15)
#else
#define STCP_MQTT_PUBLISH_INTERVAL_MS            (5 * 1000)
#endif

// Keepalive PITÄÄ olla isompi kuin poll interval.
#define STCP_MQTT_KEEPALIVE_SECONDS              300

void sleep_ms_jitter(uint32_t base_ms, uint32_t jitter_ms);
void stpc_mqtt_init_fsm_thread();
void make_timestamp(char *out, size_t max_len);
int mqtt_dns_info_free();

#if CONFIG_STCP_TESTING && (CONFIG_STCP_TESTING_MODE == 3)
#define SLEEP_JITTER_MS             50
#define SLEEP_MSEC(val)             sleep_ms_jitter(10, SLEEP_JITTER_MS)
#define SLEEP_SEC(val)              sleep_ms_jitter(20, SLEEP_JITTER_MS)
#else
#define SLEEP_JITTER_MS             500
#define SLEEP_MSEC(val)             sleep_ms_jitter(val, SLEEP_JITTER_MS)
#define SLEEP_SEC(val)              sleep_ms_jitter((val) * 1000, SLEEP_JITTER_MS)
#endif

void mqtt_client_prepare(struct mqtt_ctx *mqtt_ctx, int do_memset);
int stcp_mqtt_wait_for_connak_event(struct mqtt_client *client, struct stcp_api *api, int timeout_ms);
void stcp_mqtt_set_connak_event_seen(struct stcp_api *api);
void stcp_mqtt_reset_connak_event_seen();
#define VOID_TO_CTX(vp)    ((struct stcp_ctx*)(vp))

//
// MQTT funkkarit
//
