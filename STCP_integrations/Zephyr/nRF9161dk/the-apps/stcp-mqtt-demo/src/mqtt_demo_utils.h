#pragma once
#include <zephyr/random/random.h>
#include <zephyr/kernel.h>

#define STCP_MQTT_WAIT_CONNACK_EVENT_FOR_MSEC    (60 * 1000)
#define STCP_MQTT_POLL_TIMEOUT_MSEC              (500)

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

#if CONFIG_STCP_TESTING && (CONFIG_STCP_TESTING_MODE == 3)
#define SLEEP_JITTER_MS             50
#define SLEEP_MSEC(val)             sleep_ms_jitter(10, SLEEP_JITTER_MS)
#define SLEEP_SEC(val)              sleep_ms_jitter(20, SLEEP_JITTER_MS)
#else
#define SLEEP_JITTER_MS             500
#define SLEEP_MSEC(val)             sleep_ms_jitter(val, SLEEP_JITTER_MS)
#define SLEEP_SEC(val)              sleep_ms_jitter((val) * 1000, SLEEP_JITTER_MS)
#endif

int stcp_mqtt_wait_for_connak_event(int timeout_ms);
void stcp_mqtt_set_connak_event_seen();
void stcp_mqtt_reset_connak_event_seen();
