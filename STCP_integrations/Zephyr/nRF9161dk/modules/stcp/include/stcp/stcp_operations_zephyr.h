#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>

#include <errno.h>

#include "stcp/stcp_struct.h"
#include "stcp/stcp_net.h"
#include "stcp/stcp_operations_zephyr.h"

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif


int stcp_handshake_for_context(struct stcp_ctx *ctx);
struct stcp_ctx *stcp_create_new_context(int under);
