#include <errno.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <stcp/stcp_internal.h>

LOG_MODULE_DECLARE(stcp_offload, CONFIG_STCP_LOG_LEVEL);

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
            ctx->state = STCP_STATE_CREATED;
            k_mutex_init(&ctx->lock);
            k_sem_init(&ctx->rx_ready, 0, 1);
            k_sem_init(&ctx->connect_ready, 0, 1);
            k_fifo_init(&ctx->accept_queue);
            ring_buf_init(&ctx->rx_ring, sizeof(ctx->rx_storage), ctx->rx_storage);
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
    k_mutex_lock(&contexts_lock, K_FOREVER);
    memset(ctx, 0, sizeof(*ctx));
    ctx->fd = -1;
    ctx->state = STCP_STATE_FREE;
    k_mutex_unlock(&contexts_lock);
}
