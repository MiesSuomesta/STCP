#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <stcp/stcp_internal.h>

static int lock_ctx(struct stcp_socket_ctx *ctx)
{
    if (ctx == NULL || !ctx->allocated) {
        return -EBADF;
    }
    return k_mutex_lock(&ctx->lock, K_FOREVER);
}

static void unlock_ctx(struct stcp_socket_ctx *ctx)
{
    k_mutex_unlock(&ctx->lock);
}

int stcp_core_bind(struct stcp_socket_ctx *ctx, const struct sockaddr_in *addr)
{
    int ret;

    if (addr == NULL) return -EINVAL;
    ret = lock_ctx(ctx);
    if (ret < 0) return ret;

    if (ctx->state != STCP_STATE_CREATED) {
        unlock_ctx(ctx);
        return -EINVAL;
    }

    ret = stcp_carrier_bind(&ctx->carrier, addr);
    if (ret == 0) {
        ctx->local = *addr;
        ctx->state = STCP_STATE_BOUND;
        ctx->last_error = 0;
    } else {
        ctx->last_error = -ret;
    }

    unlock_ctx(ctx);
    return ret;
}

int stcp_core_connect(struct stcp_socket_ctx *ctx, const struct sockaddr_in *addr)
{
    enum stcp_state old_state;
    int ret;

    if (addr == NULL) return -EINVAL;
    ret = lock_ctx(ctx);
    if (ret < 0) return ret;

    if (ctx->state != STCP_STATE_CREATED && ctx->state != STCP_STATE_BOUND) {
        unlock_ctx(ctx);
        return ctx->state == STCP_STATE_CONNECTED ? -EISCONN : -EINVAL;
    }

    old_state = ctx->state;
    ctx->state = STCP_STATE_CONNECTING;
    unlock_ctx(ctx);

    /* Blocking/nonblocking behavior is inherited from the hidden carrier FD. */
    ret = stcp_carrier_connect(&ctx->carrier, addr);

    (void)lock_ctx(ctx);
    if (ret < 0) {
        ctx->state = old_state;
        ctx->last_error = -ret;
    } else {
        ctx->peer = *addr;
        ctx->state = STCP_STATE_CONNECTED;
        ctx->last_error = 0;
    }
    unlock_ctx(ctx);
    return ret;
}

int stcp_core_listen(struct stcp_socket_ctx *ctx, int backlog)
{
    int ret;

    if (backlog < 1) return -EINVAL;
    ret = lock_ctx(ctx);
    if (ret < 0) return ret;

    if (ctx->state != STCP_STATE_BOUND) {
        unlock_ctx(ctx);
        return -EINVAL;
    }

    ret = stcp_carrier_listen(&ctx->carrier, backlog);
    if (ret == 0) {
        ctx->state = STCP_STATE_LISTENING;
        ctx->last_error = 0;
    } else {
        ctx->last_error = -ret;
    }

    unlock_ctx(ctx);
    return ret;
}

int stcp_core_accept(struct stcp_socket_ctx *listener, struct stcp_socket_ctx *child,
                     struct sockaddr_in *peer, socklen_t *peer_len)
{
    int ret;

    if (listener == NULL || child == NULL) return -EINVAL;
    ret = lock_ctx(listener);
    if (ret < 0) return ret;
    if (listener->state != STCP_STATE_LISTENING) {
        unlock_ctx(listener);
        return -EINVAL;
    }
    unlock_ctx(listener);

    ret = stcp_carrier_accept(&listener->carrier, &child->carrier, peer, peer_len);
    if (ret < 0) {
        listener->last_error = -ret;
        return ret;
    }

    child->protocol = STCP_PROTO_TCP;
    child->type = SOCK_STREAM;
    child->state = STCP_STATE_CONNECTED;
    child->last_error = 0;
    if (peer != NULL) child->peer = *peer;
    return 0;
}

ssize_t stcp_core_send(struct stcp_socket_ctx *ctx, const void *buf,
                       size_t len, int flags)
{
    ssize_t ret;

    if (buf == NULL && len != 0) return -EINVAL;
    if (lock_ctx(ctx) < 0) return -EBADF;
    if (ctx->state != STCP_STATE_CONNECTED) {
        unlock_ctx(ctx);
        return -ENOTCONN;
    }
    unlock_ctx(ctx);

    ret = stcp_carrier_send(&ctx->carrier, buf, len, flags);
    if (ret < 0) ctx->last_error = (int)-ret;
    return ret;
}

ssize_t stcp_core_recv(struct stcp_socket_ctx *ctx, void *buf,
                       size_t len, int flags)
{
    ssize_t ret;

    if (buf == NULL && len != 0) return -EINVAL;
    if (lock_ctx(ctx) < 0) return -EBADF;
    if (ctx->state != STCP_STATE_CONNECTED) {
        unlock_ctx(ctx);
        return -ENOTCONN;
    }
    unlock_ctx(ctx);

    ret = stcp_carrier_recv(&ctx->carrier, buf, len, flags);
    if (ret < 0) {
        ctx->last_error = (int)-ret;
    } else if (ret == 0) {
        (void)lock_ctx(ctx);
        ctx->state = STCP_STATE_CLOSED;
        unlock_ctx(ctx);
    }
    return ret;
}

int stcp_core_shutdown(struct stcp_socket_ctx *ctx, int how)
{
    int ret;

    if (how != ZSOCK_SHUT_RD && how != ZSOCK_SHUT_WR && how != ZSOCK_SHUT_RDWR) {
        return -EINVAL;
    }
    if (lock_ctx(ctx) < 0) return -EBADF;
    if (ctx->state == STCP_STATE_CLOSED) {
        unlock_ctx(ctx);
        return 0;
    }
    if (ctx->state != STCP_STATE_CONNECTED) {
        unlock_ctx(ctx);
        return -ENOTCONN;
    }
    unlock_ctx(ctx);

    ret = stcp_carrier_shutdown(&ctx->carrier, how);
    if (ret < 0 && ret != -ENOTCONN) {
        ctx->last_error = -ret;
        return ret;
    }

    if (how == ZSOCK_SHUT_RDWR) {
        (void)lock_ctx(ctx);
        ctx->state = STCP_STATE_CLOSED;
        unlock_ctx(ctx);
    }
    return 0;
}
