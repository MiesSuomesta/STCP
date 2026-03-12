#pragma once
#include <zephyr/random/random.h>
#include <zephyr/kernel.h>

#define STCP_MQTT_WAIT_CONNACK_EVENT_FOR_MSEC    (60 * 1000)
#define STCP_MQTT_POLL_TIMEOUT_MSEC              (20 * 1000)

void sleep_ms_jitter(uint32_t base_ms, uint32_t jitter_ms);
void stpc_mqtt_init_fsm_thread();
void make_timestamp(char *out, size_t max_len);

#define SLEEP_JITTER_MS             500
#define SLEEP_MSEC(val)             sleep_ms_jitter(val, SLEEP_JITTER_MS)
#define SLEEP_SEC(val)              sleep_ms_jitter((val) * 1000, SLEEP_JITTER_MS)

int stcp_mqtt_wait_for_connak_event(int timeout_ms);
void stcp_mqtt_set_connak_event_seen();
void stcp_mqtt_reset_connak_event_seen();
