#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>

#include <errno.h>

#include <stcp_api.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_rust_exported_functions.h>

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif


int stcp_handshake_for_context(struct stcp_ctx *ctx);
struct stcp_ctx *stcp_create_new_context(int under);

int stcp_ctx_ref_count_get(struct stcp_ctx *ctx);
int stcp_ctx_ref_count_is_what(struct stcp_ctx *ctx);
int stcp_ctx_ref_count_put(struct stcp_ctx *ctx);

void stcp_create_init_new_context(struct stcp_ctx *ctx);
int stcp_wait_for_handshake_signal(struct stcp_ctx *ctx);
