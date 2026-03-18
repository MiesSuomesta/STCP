#pragma once
#include <zephyr/kernel.h>
#include <stdbool.h>

#define STCP_SOCKET_INTERNAL 1
#include <stcp/settings.h>
#include <stcp/debug.h>
#include <stcp/stcp_alloc.h>
#include <stcp/stcp_struct.h>
#include <stcp/utils.h>
#include <stcp/stcp_net.h>
#include <stcp/fsm.h>
#include <stcp/workers.h>
#include <stcp/stcp_operations_zephyr.h>
#include <stcp/stcp_rx_transmission.h>

struct stcp_api {
    struct stcp_ctx *ctx;
    int nonblocking;
};

