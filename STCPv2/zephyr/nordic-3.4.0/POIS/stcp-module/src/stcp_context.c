#include <string.h>
#include <zephyr/kernel.h>
#include <stcp/stcp_internal.h>

static struct stcp_socket_ctx contexts[CONFIG_STCP_MAX_SOCKETS];
K_MUTEX_DEFINE(contexts_lock);

struct stcp_socket_ctx *stcp_ctx_alloc(void)
{
    struct stcp_socket_ctx *ctx = NULL;

    k_mutex_lock(&contexts_lock, K_FOREVER);
    for (size_t i = 0; i < ARRAY_SIZE(contexts); ++i) {
        if (!contexts[i].allocated) {
            ctx = &contexts[i];
            memset(ctx, 0, sizeof(*ctx));
            ctx->allocated = true;
            ctx->fd = -1;
            ctx->carrier.fd = -1;
            ctx->state = STCP_STATE_CREATED;
            k_mutex_init(&ctx->lock);
            break;
        }
    }
    k_mutex_unlock(&contexts_lock);
    return ctx;
}

void stcp_ctx_release(struct stcp_socket_ctx *ctx)
{
    if (ctx == NULL) {
        return;
    }

    stcp_carrier_close(&ctx->carrier);

    k_mutex_lock(&contexts_lock, K_FOREVER);
    memset(ctx, 0, sizeof(*ctx));
    ctx->fd = -1;
    ctx->carrier.fd = -1;
    ctx->state = STCP_STATE_FREE;
    k_mutex_unlock(&contexts_lock);
}
