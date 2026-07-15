#ifndef STCP_RUST_FFI_H
#define STCP_RUST_FFI_H

#include <linux/types.h>

int stcp_rust_init(void);
int stcp_rust_crypto_selftest(void);
void stcp_rust_exit(void);

void *stcp_rust_create(u8 proto);
void stcp_rust_release(void *ctx);
void stcp_rust_set_owner(void *ctx, void *owner);

int stcp_rust_bind(void *ctx, u32 addr, u16 port);
int stcp_rust_listen(void *ctx, int backlog);
int stcp_rust_connect(void *ctx, u32 addr, u16 port, int flags);
int stcp_rust_accept(void *ctx, void **accepted_ctx, int flags);

ssize_t stcp_rust_send(void *ctx, const u8 *buffer, size_t len, int flags);
ssize_t stcp_rust_recv(void *ctx, u8 *buffer, size_t len, int flags);

int stcp_rust_has_accept(void *ctx);
int stcp_rust_has_data(void *ctx);
int stcp_rust_is_connected(void *ctx);

void stcp_rust_shutdown(void *ctx, int how);

#endif
