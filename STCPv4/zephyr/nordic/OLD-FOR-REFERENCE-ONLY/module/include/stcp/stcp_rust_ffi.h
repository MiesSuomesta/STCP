#ifndef STCP_RUST_FFI_ZEPHYR_H
#define STCP_RUST_FFI_ZEPHYR_H
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#ifdef __cplusplus
extern "C" {
#endif
struct stcp_reliability_stats {
    uint32_t srtt_ms, rttvar_ms, rto_ms;
    uint64_t sent_frames, acknowledged_frames, retransmitted_frames;
    uint64_t duplicate_frames, reordered_frames, timeout_failures, rtt_samples;
};
int stcp_rust_init(void);
void stcp_rust_exit(void);
int stcp_rust_create(uint8_t proto, void **out_ctx);
void stcp_rust_release(void *ctx);
void stcp_rust_set_owner(void *ctx, void *owner);
void stcp_rust_set_carrier(void *ctx, void *carrier);
void *stcp_rust_get_carrier(void *ctx);
int stcp_rust_bind(void *ctx, uint32_t addr, uint16_t port);
int stcp_rust_listen(void *ctx, int backlog);
int stcp_rust_connect(void *ctx, uint32_t addr, uint16_t port, int flags);
int stcp_rust_start_handshake(void *ctx);
int stcp_rust_accept(void *ctx, void **out_ctx, int flags);
int stcp_rust_has_accept(void *ctx);
ssize_t stcp_rust_send(void *ctx, const uint8_t *data, size_t len, int flags);
ssize_t stcp_rust_recv(void *ctx, uint8_t *data, size_t len, int flags);
int stcp_rust_has_data(void *ctx);
int stcp_rust_is_connected(void *ctx);
int stcp_rust_can_send(void *ctx, size_t len);
int stcp_rust_tick(void *ctx);
void stcp_rust_shutdown(void *ctx, int how);
int stcp_rust_get_reliability_stats(void *ctx, struct stcp_reliability_stats *out);
int stcp_rust_crypto_selftest(void);
int stcp_rust_carrier_receive(void *ctx, const uint8_t *data, size_t len);
int stcp_rust_carrier_receive_from(void *ctx, const uint8_t *data, size_t len,
                                   uint32_t peer_addr, uint16_t peer_port);
#ifdef __cplusplus
}
#endif
#endif
