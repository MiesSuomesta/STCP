#pragma once
#include <zephyr/logging/log.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>

#include <stcp_api.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_socket.h>

#include <stcp/debug.h>

#if CONFIG_MQTT_LIB_STCP

int mqtt_client_stcp_connect(struct mqtt_client *client);
int mqtt_client_stcp_write(struct mqtt_client *client, const uint8_t *data,
			  uint32_t datalen);
int mqtt_client_stcp_write_msg(struct mqtt_client *client,
			      const struct msghdr *message);
int mqtt_client_stcp_read(struct mqtt_client *client, uint8_t *data, uint32_t buflen,
			 bool shall_block);
int mqtt_client_stcp_disconnect(struct mqtt_client *client);

#endif
