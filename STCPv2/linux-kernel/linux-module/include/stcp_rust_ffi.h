#ifndef STCP_RUST_FFI_H
#define STCP_RUST_FFI_H

#include <linux/types.h>

int stcp_rust_init(void);
void stcp_rust_exit(void);

/* Current Rust ABI: returns errno and writes the context through out_ctx. */
int stcp_rust_create(u8 proto, void **out_ctx);
void stcp_rust_release(void *ctx);

void stcp_rust_set_owner(void *ctx, void *owner);
void stcp_rust_set_carrier(void *ctx, void *carrier);
void *stcp_rust_get_carrier(void *ctx);

int stcp_rust_has_data(void *ctx);
int stcp_rust_is_connected(void *ctx);

int stcp_rust_bind(void *ctx, u32 addr, u16 port);
int stcp_rust_listen(void *ctx, int backlog);
int stcp_rust_connect(void *ctx, u32 addr, u16 port, int flags);
int stcp_rust_start_handshake(void *ctx);
int stcp_rust_accept(void *ctx, void **out_ctx, int flags);
int stcp_rust_has_accept(void *ctx);

ssize_t stcp_rust_send(void *ctx, const u8 *data, size_t len, int flags);
ssize_t stcp_rust_recv(void *ctx, u8 *data, size_t len, int flags);
void stcp_rust_shutdown(void *ctx, int how);
int stcp_rust_tick(void *ctx);
int stcp_rust_crypto_selftest(void);

int stcp_rust_carrier_receive(void *ctx, const u8 *data, size_t len);
int stcp_rust_carrier_receive_from(void *ctx, const u8 *data, size_t len, u32 peer_addr, u16 peer_port);
int stcp_rust_get_udp_peer(void *ctx, u32 *out_addr, u16 *out_port);

#endif
