#ifndef STCP_RUST_H
#define STCP_RUST_H

#include <linux/types.h>

int stcp_rust_init(void);
void stcp_rust_exit(void);
void *stcp_rust_create(u8 proto_id);
void stcp_rust_release(void *ctx);
int stcp_rust_bind(void *ctx, __be32 addr, __be16 port);
int stcp_rust_listen(void *ctx, int backlog);
int stcp_rust_connect(void *ctx, __be32 addr, __be16 port, int flags);
int stcp_rust_accept(void *listener, void **accepted_ctx, int flags);
ssize_t stcp_rust_send(void *ctx, const u8 *buf, size_t len, int flags);
ssize_t stcp_rust_recv(void *ctx, u8 *buf, size_t len, int flags);
void stcp_rust_shutdown(void *ctx, int how);

#endif
