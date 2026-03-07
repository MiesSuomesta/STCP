#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>
#include <modem/lte_lc.h>
#include <zephyr/logging/log.h>

#include <stcp_api.h>

int stpc_mqtt_subscribe(struct mqtt_client *client);

int mqtt_connect_via_stcp(char *host, char *port, struct stcp_api **saveTo);

int mqtt_server_stcp_recv_loop(struct stcp_api *api);

int mqtt_publish_message(const char *topic,
                         const uint8_t *payload,
                         size_t len);
