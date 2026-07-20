#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <stcp/stcp_internal.h>

LOG_MODULE_DECLARE(stcp_offload, CONFIG_STCP_LOG_LEVEL);

int stcp_core_bind(struct stcp_socket_ctx *ctx, const struct sockaddr_stcp *addr)
{
    if (ctx == NULL || addr == NULL) return -EINVAL;
    ctx->local = *addr;
    ctx->state = STCP_STATE_BOUND;
    return 0;
}

int stcp_core_listen(struct stcp_socket_ctx *ctx, int backlog)
{
    if (ctx == NULL || backlog < 1) return -EINVAL;
    if (ctx->state != STCP_STATE_BOUND) return -EINVAL;
    ctx->backlog = MIN(backlog, CONFIG_STCP_ACCEPT_BACKLOG);
    ctx->state = STCP_STATE_LISTENING;
    return 0;
}

int stcp_core_connect(struct stcp_socket_ctx *ctx, const struct sockaddr_stcp *addr)
{
    if (ctx == NULL || addr == NULL) return -EINVAL;
    if (ctx->state != STCP_STATE_CREATED && ctx->state != STCP_STATE_BOUND) return -EISCONN;
    ctx->peer = *addr;
    ctx->state = STCP_STATE_CONNECTING;
    /* Stub behavior: mark connected immediately. Replace with handshake later. */
    ctx->state = STCP_STATE_CONNECTED;
    k_sem_give(&ctx->connect_ready);
    return 0;
}

ssize_t stcp_core_send(struct stcp_socket_ctx *ctx, const void *buf, size_t len, int flags)
{
    ARG_UNUSED(flags);
    if (ctx == NULL || (buf == NULL && len != 0)) return -EINVAL;
    if (ctx->state != STCP_STATE_CONNECTED) return -ENOTCONN;
    /* Stub accepts the bytes but deliberately does not transport them. */
    return (ssize_t)len;
}

ssize_t stcp_core_recv(struct stcp_socket_ctx *ctx, void *buf, size_t len, int flags)
{
    uint32_t got;
    if (ctx == NULL || (buf == NULL && len != 0)) return -EINVAL;
    got = ring_buf_get(&ctx->rx_ring, buf, len);
    if (got != 0) return (ssize_t)got;
    if (ctx->state == STCP_STATE_CLOSED) return 0;
    if (ctx->nonblocking || (flags & ZSOCK_MSG_DONTWAIT)) return -EAGAIN;
    return -EAGAIN; /* No carrier exists yet, so never block forever in the stub. */
}

int stcp_core_shutdown(struct stcp_socket_ctx *ctx, int how)
{
    ARG_UNUSED(how);
    if (ctx == NULL) return -EINVAL;
    ctx->state = STCP_STATE_CLOSED;
    k_sem_give(&ctx->rx_ready);
    k_sem_give(&ctx->connect_ready);
    return 0;
}
