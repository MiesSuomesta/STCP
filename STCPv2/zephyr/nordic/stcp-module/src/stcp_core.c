#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <stcp/stcp_internal.h>

int stcp_core_bind(struct stcp_socket_ctx *ctx, const struct sockaddr_in *addr)
{
    int ret;
    if (ctx == NULL || addr == NULL) return -EINVAL;
    if (ctx->state != STCP_STATE_CREATED) return -EINVAL;
    ret = stcp_carrier_bind(&ctx->carrier, addr);
    if (ret < 0) return ret;
    ctx->local = *addr;
    ctx->state = STCP_STATE_BOUND;
    return 0;
}

int stcp_core_connect(struct stcp_socket_ctx *ctx, const struct sockaddr_in *addr)
{
    int ret;
    if (ctx == NULL || addr == NULL) return -EINVAL;
    if (ctx->state != STCP_STATE_CREATED && ctx->state != STCP_STATE_BOUND) return -EISCONN;
    ctx->state = STCP_STATE_CONNECTING;
    ret = stcp_carrier_connect(&ctx->carrier, addr);
    if (ret < 0) {
        ctx->state = STCP_STATE_CREATED;
        return ret;
    }
    ctx->peer = *addr;
    ctx->state = STCP_STATE_CONNECTED;
    return 0;
}

int stcp_core_listen(struct stcp_socket_ctx *ctx, int backlog)
{
    int ret;
    if (ctx == NULL || backlog < 1) return -EINVAL;
    if (ctx->state != STCP_STATE_BOUND) return -EINVAL;
    ret = stcp_carrier_listen(&ctx->carrier, backlog);
    if (ret < 0) return ret;
    ctx->state = STCP_STATE_LISTENING;
    return 0;
}

int stcp_core_accept(struct stcp_socket_ctx *listener, struct stcp_socket_ctx *child,
                     struct sockaddr_in *peer, socklen_t *peer_len)
{
    int ret;
    if (listener == NULL || child == NULL) return -EINVAL;
    ret = stcp_carrier_accept(&listener->carrier, &child->carrier, peer, peer_len);
    if (ret < 0) return ret;
    child->protocol = STCP_PROTO_TCP;
    child->type = SOCK_STREAM;
    child->state = STCP_STATE_CONNECTED;
    if (peer != NULL) child->peer = *peer;
    return 0;
}

ssize_t stcp_core_send(struct stcp_socket_ctx *ctx, const void *buf,
                       size_t len, int flags)
{
    if (ctx == NULL) return -EINVAL;
    if (ctx->state != STCP_STATE_CONNECTED) return -ENOTCONN;
    return stcp_carrier_send(&ctx->carrier, buf, len, flags);
}

ssize_t stcp_core_recv(struct stcp_socket_ctx *ctx, void *buf,
                       size_t len, int flags)
{
    if (ctx == NULL) return -EINVAL;
    if (ctx->state != STCP_STATE_CONNECTED) return -ENOTCONN;
    return stcp_carrier_recv(&ctx->carrier, buf, len, flags);
}

int stcp_core_shutdown(struct stcp_socket_ctx *ctx, int how)
{
    int ret;
    if (ctx == NULL) return -EINVAL;
    if (ctx->state == STCP_STATE_CLOSED) return 0;
    ret = stcp_carrier_shutdown(&ctx->carrier, how);
    if (ret < 0 && ret != -ENOTCONN) return ret;
    ctx->state = STCP_STATE_CLOSED;
    return 0;
}
