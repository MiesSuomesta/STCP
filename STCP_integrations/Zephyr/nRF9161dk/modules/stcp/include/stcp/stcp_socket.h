#pragma once
#include <stcp_api.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_socket.h>
#include <stddef.h>
#include <stdint.h>

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

#ifdef STCP_SOCKET_INTERNAL
int     stcp_new_context_with_fd(struct stcp_ctx **ctxSaveTo, int fd);
int     stcp_new_empty_context(struct stcp_ctx **ctxSaveTo);
int     stcp_new_context(struct stcp_ctx **ctxSaveTo);
int     stcp_connect(struct stcp_ctx *ctx, const struct sockaddr *addr, socklen_t addrlen);
ssize_t stcp_send(struct stcp_ctx *ctx, const void *buf, size_t len, int flags);
ssize_t stcp_send_msg(struct stcp_ctx *ctx, const struct msghdr *message);
ssize_t stcp_recv(struct stcp_ctx *ctx, void *buf, size_t len, int flags);
int     stcp_close(struct stcp_ctx *ctx);

int stcp_accept(struct stcp_ctx *parent,
                struct stcp_ctx **child_out,
                struct sockaddr *peer_addr,
                socklen_t *peer_len);
                
int stcp_listen(struct stcp_ctx *ctx,
                int backlog);          

int stcp_bind(struct stcp_ctx *ctx,
              const struct sockaddr *addr,
              socklen_t addrlen);
int stcp_socket_poll(struct zsock_pollfd *fds, int nfds, int timeout);
int stcp_set_non_bloking_to(struct stcp_ctx *ctx, int val);
#endif
