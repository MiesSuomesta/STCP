#ifndef STCP_RUST_FFI_H
#define STCP_RUST_FFI_H

#include <linux/types.h>


struct stcp_compression_stats;


struct stcp_reliability_stats {
	u32 srtt_ms;
	u32 rttvar_ms;
	u32 rto_ms;
	u64 sent_frames;
	u64 acknowledged_frames;
	u64 retransmitted_frames;
	u64 duplicate_frames;
	u64 reordered_frames;
	u64 timeout_failures;
	u64 rtt_samples;
};

int stcp_rust_init(void);
void stcp_rust_exit(void);

/* Current Rust ABI: returns errno and writes the context through out_ctx. */
int stcp_rust_create(u8 proto, void **out_ctx);
void stcp_rust_release(void *ctx);

void stcp_rust_set_owner(void *ctx, void *owner);
void stcp_rust_set_carrier(void *ctx, void *carrier);
void *stcp_rust_get_carrier(void *ctx);
int stcp_rust_set_compression(void *ctx, int enabled);
int stcp_rust_set_compression_threshold(void *ctx, u32 threshold);

int stcp_rust_has_data(void *ctx);
int stcp_rust_is_connected(void *ctx);
int stcp_rust_can_send(void *ctx, size_t len);

int stcp_rust_bind(void *ctx, u32 addr, u16 port);
int stcp_rust_listen(void *ctx, int backlog);
int stcp_rust_connect(void *ctx, u32 addr, u16 port, int flags);
int stcp_rust_start_handshake(void *ctx);
int stcp_rust_accept(void *ctx, void **out_ctx, int flags);
int stcp_rust_create_external_tcp_child(
	void *listener_ctx,
	u32 local_addr,
	u16 local_port,
	u32 peer_addr,
	u16 peer_port,
	void **out_ctx
);
u64 stcp_rust_connection_id(void *ctx);
int stcp_rust_has_accept(void *ctx);

ssize_t stcp_rust_send(void *ctx, const u8 *data, size_t len, int flags);
ssize_t stcp_rust_recv(void *ctx, u8 *data, size_t len, int flags);
void stcp_rust_shutdown(void *ctx, int how);
int stcp_rust_tick(void *ctx);
int stcp_rust_get_reliability_stats(void *ctx, struct stcp_reliability_stats *out_stats);
int stcp_rust_get_compression_stats(void *ctx, struct stcp_compression_stats *out_stats);
int stcp_rust_crypto_selftest(void);

/* Native STCP_P2P shared-core status / wire-codec selftest. */
u32 stcp_p2p_core_abi_version(void);
int stcp_p2p_core_selftest(void);
int stcp_p2p_core_ready(void);
int stcp_p2p_noise_core_ready(void);
int stcp_p2p_noise_selftest(void);
int stcp_p2p_noise_dialer_new(const u8 identity_seed[32], void **out_ctx);
void stcp_p2p_noise_dialer_free(void *ctx);
int stcp_p2p_noise_dialer_message1(void *ctx, u8 *out, size_t cap);
int stcp_p2p_noise_dialer_message2(void *ctx, const u8 *msg2, size_t msg2_len, u8 *out3, size_t cap);
int stcp_p2p_noise_dialer_complete(void *ctx);
int stcp_p2p_noise_listener_new(const u8 identity_seed[32], void **out_ctx);
void stcp_p2p_noise_listener_free(void *ctx);
int stcp_p2p_noise_listener_message1(void *ctx, const u8 *msg1, size_t msg1_len, u8 *out2, size_t cap);
int stcp_p2p_noise_listener_message3(void *ctx, const u8 *msg3, size_t msg3_len);
int stcp_p2p_noise_listener_complete(void *ctx);
const u8 *stcp_p2p_core_stage_name(u32 stage);

int stcp_rust_carrier_receive(void *ctx, const u8 *data, size_t len);
int stcp_rust_carrier_receive_from(void *ctx, const u8 *data, size_t len, u32 peer_addr, u16 peer_port);
int stcp_rust_get_udp_peer(void *ctx, u32 *out_addr, u16 *out_port);

#endif

