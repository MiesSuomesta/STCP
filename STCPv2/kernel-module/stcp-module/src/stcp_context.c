#include <string.h>
#include <zephyr/kernel.h>
#include <stcp/stcp_internal.h>
static struct stcp_ctx pool[CONFIG_STCP_MAX_SOCKETS];
static K_MUTEX_DEFINE(pool_lock);
struct stcp_ctx *stcp_ctx_alloc(void)
{
    struct stcp_ctx *ret = NULL;
    k_mutex_lock(&pool_lock, K_FOREVER);
    for (size_t i = 0; i < ARRAY_SIZE(pool); ++i) {
        if (!pool[i].used) {
            memset(&pool[i], 0, sizeof(pool[i]));
            pool[i].used = true;
            pool[i].fd = -1;
            pool[i].carrier_fd = -1;
            pool[i].state = STCP_CREATED;
            k_mutex_init(&pool[i].lock);
            ret = &pool[i];
            break;
        }
    }
    k_mutex_unlock(&pool_lock);
    return ret;
}
void stcp_ctx_free(struct stcp_ctx *ctx)
{
    if (!ctx) return;
    k_mutex_lock(&pool_lock, K_FOREVER);
    memset(ctx, 0, sizeof(*ctx));
    ctx->fd = -1;
    ctx->carrier_fd = -1;
    k_mutex_unlock(&pool_lock);
}
