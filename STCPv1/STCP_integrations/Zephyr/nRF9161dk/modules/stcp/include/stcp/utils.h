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

#define STCP_INTERNAL

#include "stcp/stcp_rust_exported_functions.h"
#include <stcp/stcp_struct.h>

#define STCP_USE_LTE			1
#include "stcp/debug.h"

// LTE / BT kytkin ...
#define TEST_CONNECTON_TO_HOST 	"lja.fi"
#define TEST_CONNECTON_TO_PORT 	"7777"
#define STCP_WAIT_IN_SECONDS    180

#define STCP_HEAP_DEBUG         0

//void sleep_with_jitter(uint32_t base_ms, uint32_t jitter_ms);

void sleep_ms_jitter(uint32_t base_ms, uint32_t jitter_ms);

void stcp_util_log_sockaddr(char *tag, const struct zsock_addrinfo *ai);

int stcp_context_set_target(struct stcp_ctx *ctx, const char *pHost, const char *pPort);

int stcp_util_hostname_resolver(const char *host, const char *port, struct zsock_addrinfo **result);

struct addrinfo *resolve_to_connect(const char *host, const char *port);
int              stcp_tcp_resolve_and_make_socket(const char *host, const char *port);

struct stcp_ctx *stcp_tcp_resolve_and_make_context(const char *host, const char *port);
int              stcp_tcp_do_hanshake_with_context(struct stcp_ctx *ctx);

int stcp_tcp_context_connect_and_shake_hands(struct stcp_ctx *ctx, int timeout_ms);

int stcp_mqtt_connect_via_stcp(int fd);

int stcp_config_debug_enabled();
int stcp_config_aes_bypass_enabled();

int stcp_exported_rust_ctx_alive_count(void);

void stcp_hexdump_ascii(const char *prefix, const uint8_t *buf, int len);

int stcp_is_context_valid(void *vpCtx);
void stcp_sleep_ms(uint32_t ms);
int stcp_tcp_timeout_set_to_fd(int fd, int timeout_ms);

int stcp_is_file_desc_alive(int fd);
int stcp_get_pending_fd_error(int fd);

void dump_socket_error(int fd);

