#include <errno.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <modem/lte_lc.h>
#include <modem/pdn.h>

#include <zephyr/net/socket.h>
#include <zephyr/posix/poll.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/sys/atomic.h>
#include <modem/nrf_modem_lib.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(stcp_workers, LOG_LEVEL_INF);

#include "stcp/debug.h"
#include "stcp/stcp_alloc.h"
#include "stcp/stcp_struct.h"
#include "stcp/utils.h"
#include "stcp/stcp_net.h"
#include "stcp/workers.h"
#include "stcp/stcp_operations_zephyr.h"
#include "stcp/stcp_rust_exported_functions.h"

K_THREAD_STACK_DEFINE(stcp_worker_stack, (4096*2));
static struct k_work_q stcp_work_q;

#define STCP_DO_FREE_FOR_RESOURCES 0

static int stcp_workers_init(const struct device *dev)
{
    ARG_UNUSED(dev);
    k_work_queue_start(&stcp_work_q,
                       stcp_worker_stack,
                       K_THREAD_STACK_SIZEOF(stcp_worker_stack),
                       5,   // priority
                       NULL);

    LOG_INF("STCP worker queue started");
    return 0;
}

SYS_INIT(stcp_workers_init, APPLICATION, 90);

void worker_context_init(struct stcp_ctx *ctx) {
    LDBG("Init of worker (NOP)");
//    atomic_set(&ctx->closing, 0);
//    k_work_init(&ctx->cleanup_work, worker_cleanup_work_handler);
//    LDBG("Init of worker, DONE");
}

void worker_cleanup_work_handler(struct k_work *work)
{
    struct stcp_ctx *ctx =
        CONTAINER_OF(work, struct stcp_ctx, cleanup_work);

    if (!ctx) {
        LDBG("STCP cleanup, no context...");
        return ;
    }

    if (!ctx->handshake_done) {
        LDBG("STCP cleanup disabled for %p, No HS done", ctx);
        return ;
    }

    LDBG("STCP cleanup running for %p\n", ctx);

#if STCP_DO_FREE_FOR_RESOURCES
    /* 2️⃣ Tuhotaan Rust sessio */
    if (ctx->session) {
        rust_exported_session_destroy(ctx->session);
        ctx->session = NULL;
    }

#ifdef CONFIG_STCP_DEBUG_POISON
    /* 3️⃣ Debug poison */
    memset(ctx, 0xDE, sizeof(*ctx));
#endif

    /* 1️⃣ Sulje socket */
    if (ctx->ks.fd >= 0) {
        stcp_net_close_fd(&ctx->ks.fd);
    }

    /* 4️⃣ Vapautus */
    k_free(ctx);
    LDBG("STCP cleanup done (Free version)\n");
#else 
    /* 1️⃣ Sulje socketit */

    if (ctx->ks.fd >= 0) {
        stcp_net_close_fd(&ctx->ks.fd);
    }
    LDBG("STCP cleanup done (No free version)\n");
#endif

    LDBG("STCP Cleanup: Resetting everything...\n");
    stcp_lte_reset_everythign(ctx);

}

void worker_schedule_cleanup(struct stcp_ctx * ctx) {
    LDBG("Cleanup %p ?", ctx);
    if (ctx != NULL) {
        LDBG("Cleanup for %p scheduled..(NOP)", ctx);
        //atomic_set(&ctx->closing, 1);
        /* Aikatauluta cleanup */
        //k_work_submit_to_queue(&stcp_work_q, &ctx->cleanup_work);
    }
}

