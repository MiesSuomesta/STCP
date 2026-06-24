/*
* STCP socket dispatcher for Zephyr / NCS 2.9 (Zephyr 3.7)
* STCP rides on TCP underneath (for now)
*/

#include <zephyr/logging/log.h>

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>

#include <errno.h>

#include <stcp/debug.h>
#include <stcp/stcp_struct.h>
#include <stcp/fsm.h>
#include <stcp/utils.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/stcp_rust_exported_functions.h>
#include <stcp/stcp_alloc.h>
#include <stcp/dns.h>
#include <stcp/low_level_pointer.h>

#include <stcp/low_level_refcount_tracker.h>

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

#define STCP_LIFESPAN_INIT_VALUE            1
extern atomic_t stcp_context_alive_count;

void stcp_lifespan_set_api_alive(struct stcp_api *api, int val);
void stcp_lifespan_set_api_status(struct stcp_api *api, int val);

int stcp_context_do_assert_check(void *pVP);
int stcp_context_lifespan_extend(struct stcp_ctx *ctx);
int stcp_context_lifespan_get_span(struct stcp_ctx *ctx);
int stcp_context_lifespan_shorten(struct stcp_ctx *ctx);