#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <stdbool.h>
#include <errno.h>
#include <zephyr/random/random.h>

#include <stcp/stcp_api.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/stcp_tcp_low_level_operations.h>
#include <stcp/stcp_struct.h>
#include <stcp/debug.h>

int stcp_dns_resolve_stcp_host(const char *pHost, const int pPort, struct zsock_addrinfo **res);
int stcp_dns_resolve_stcp_context(struct stcp_ctx *ctx, struct zsock_addrinfo **out);
int stcp_dns_resolve(const char *pHost, const int pPort, struct zsock_addrinfo **res);
int stcp_dns_free(struct zsock_addrinfo *ptr);
int stcp_dns_resolve_testing_target();
int stcp_dns_resolve_stcp_target();
int stcp_dns_free_testing_target();
int stcp_dns_free_stcp_target();
int stcp_dns_resolve_all();
