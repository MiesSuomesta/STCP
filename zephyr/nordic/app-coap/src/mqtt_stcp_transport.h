#ifndef MQTT_STCP_TRANSPORT_H
#define MQTT_STCP_TRANSPORT_H

#include <zephyr/net/mqtt.h>

struct mqtt_stcp_transport_data {
    int fd;
};

int mqtt_stcp_poll_fd(const struct mqtt_client *client);

#endif
