#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/init.h>
#include <modem/lte_lc.h>

#include <zephyr/sys/heap_listener.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/kernel.h>

#include <zephyr/net/socket.h>
#include <zephyr/net/net_ip.h>
#include <errno.h>
#include <zephyr/sys/atomic.h>
#include <modem/nrf_modem_lib.h>

#include "stcp_struct.h"
#include "stcp_net.h"
#include "workers.h"
#include "stcp_bridge.h"
#include "stcp_operations_zephyr.h"

#include "stcp_rust_exported_functions.h"

#include <stcp.h>

#define STCP_USE_LTE			1
#include "debug.h"

// LTE / BT kytkin ...
#define TEST_CONNECTON_TO_HOST 	"lja.fi"
#define TEST_CONNECTON_TO_PORT 	"7777"
#define STCP_WAIT_IN_SECONDS    180

#define STCP_HEAP_DEBUG         0

#include "stcp_transport.h"
#include "stcp_net.h"
#include "stcp_operations_zephyr.h"
#include "stcp_platform.h"
#include "stcp_transport.h"

struct addrinfo *resolve_to_connect(const char *host, const char *port);
int              stcp_tcp_resolve_and_make_socket(const char *host, const char *port);

struct stcp_ctx *stcp_tcp_resolve_and_make_context(const char *host, const char *port);
int              stcp_tcp_do_hanshake_with_context(struct stcp_ctx *ctx);

int stcp_tcp_context_connect_and_shake_hands(struct stcp_ctx *ctx, int timeout_ms);


int stcp_config_debug_enabled();
int stcp_config_aes_bypass_enabled();