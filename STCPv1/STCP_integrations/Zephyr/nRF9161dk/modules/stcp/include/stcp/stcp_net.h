#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stcp/stcp_struct.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

#define STCP_NET_FLAGS_POLL_RX      1

// kutsu mainista kun sock luotu
void stcp_net_set_sock(int sock);
int stcp_net_send(struct stcp_ctx *ctx, const uint8_t *buf, size_t len);
int stcp_net_recv(struct stcp_ctx *ctx, uint8_t *buf, size_t max_len, int flags);



#ifdef __cplusplus
}
#endif
