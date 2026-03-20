#pragma once
#include <stdint.h>

struct mqtt_stcp_stats {
    uint64_t tx_bytes;
    uint64_t rx_bytes;

    uint32_t tx_calls;
    uint32_t rx_calls;

    uint32_t tx_eagain;
    uint32_t rx_eagain;

    uint32_t reconnects;
    uint32_t connects;
};

void mqtt_stats_dump(void);