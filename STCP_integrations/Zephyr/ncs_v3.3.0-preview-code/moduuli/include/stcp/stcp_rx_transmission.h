#pragma once
#include <stcp_api.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_socket.h>
#include <stdint.h>

void stcp_context_recv_stream_init(struct stcp_ctx *ctx);
int stcp_recv_frame(struct stcp_ctx *ctx);
