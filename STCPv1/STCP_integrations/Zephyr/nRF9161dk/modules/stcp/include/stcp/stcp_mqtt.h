#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/mqtt.h>
#include <modem/lte_lc.h>
#include <zephyr/logging/log.h>

int mqtt_connect_via_stcp(int fd);
