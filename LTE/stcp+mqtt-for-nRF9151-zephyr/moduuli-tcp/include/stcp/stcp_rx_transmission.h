#pragma once
#include <stcp/stcp_api.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_socket.h>
#include <stdint.h>
#include <stcp/low_level_operations.h>

int stcp_context_recv_stream_init(struct stcp_ctx *ctx);
int stcp_recv_frame(struct stcp_ctx *ctx);
