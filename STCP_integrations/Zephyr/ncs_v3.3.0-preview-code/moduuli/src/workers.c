#include <errno.h>
#include <modem/lte_lc.h>
#include <modem/pdn.h>

#include <zephyr/net/socket.h>
#include <zephyr/posix/poll.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/sys/atomic.h>
#include <modem/nrf_modem_lib.h>

#include <zephyr/logging/log.h>

#include <stcp_api.h>
#include <stcp/debug.h>
#include <stcp/stcp_alloc.h>
#include <stcp/stcp_struct.h>
#include <stcp/utils.h>
#include <stcp/stcp_net.h>
#include <stcp/workers.h>
#include <stcp/stcp_operations_zephyr.h>
#include <stcp/stcp_rust_exported_functions.h>
#include <stcp/stcp_api_internal.h>

#include <stcp/dns.h>

K_THREAD_STACK_DEFINE(stcp_worker_stack, (4096));
static struct k_work_q stcp_work_cleanup_q;

int rust_session_is_valid(void *sess);

#define CTX_SOCK_LOCK(ctx) \
    do {                                       \
        LDBG("Locking %p context...", ctx);    \
        k_mutex_lock(&(ctx->lock), K_FOREVER); \
        LDBG("Locked %p context...", ctx);     \
    } while (0)

#define CTX_SOCK_UNLOCK(ctx) \
    do {                                       \
        LDBG("Unlocking %p context...", ctx);  \
        k_mutex_unlock(&(ctx->lock));          \
        LDBG("Unlocked %p context...", ctx);   \
    } while (0)


#define CONFIG_STCP_DEBUG_POISON        1

static int stcp_workers_init(void)
{
    k_work_queue_start(&stcp_work_cleanup_q,
                       stcp_worker_stack,
                       K_THREAD_STACK_SIZEOF(stcp_worker_stack),
                       5,   // priority
                       NULL);

    LINF("STCP worker queue started");
    return 0;
}

SYS_INIT(stcp_workers_init, APPLICATION, 90);

void worker_context_init(struct stcp_ctx *ctx) {
    LDBG("Init of worker..");
    k_work_init(&ctx->cleanup_work, worker_cleanup_work_handler);
    LDBG("Init of worker, DONE");
}

void worker_cleanup_work_handler(struct k_work *work)
{
    LDBG("STCP cleanup staring running for %p\n", work);
	LDBG("At worker %p:", work);	

    struct stcp_ctx *ctx =
        CONTAINER_OF(work, struct stcp_ctx, cleanup_work);
    LDBG("WORK=%p\n", work);
    LDBG("CTX=%p\n", ctx);
    LDBG("&cleanup_work=%p\n", &ctx->cleanup_work);

    if (ctx->magic != STCP_CTX_MAGIC_ALIVE) {
        LERR("CTX not initialized yet %p magic=%x\n", ctx, ctx->magic);
        return;
    }

    LDBG("STCP cleanup context running for %p\n", ctx);

    if ((uintptr_t)ctx < 0x20000000) {
        LERR("CTX POINTER INVALID %p\n", ctx);
        return;
    }

    if (!ctx) {
        LDBG("STCP cleanup, no context...");
        return ;
    }

    if (!atomic_cas(&ctx->closing, 0, 1)) {
        LWRN("Context already in cleaning up....");
        return;   // joku muu jo siivoaa
    }

    if ( ctx->magic == STCP_CTX_MAGIC_POISON ) {
        LWRN("Context magic is poisoned already");
        return;
    }

    LINF("Cleanup fetching pointers....");
    CTX_SOCK_LOCK(ctx);
    LINF("Cleanup fetching pointers in guarded zone");

    if (ctx->magic != STCP_CTX_MAGIC_ALIVE) {
        CTX_SOCK_UNLOCK(ctx);
        return;
    }

    ctx->magic = STCP_CTX_MAGIC_POISON;

    void *session = ctx->session;
    struct stcp_api *theAPI = ctx->api;
    int sockFD = ctx->ks.fd;

    ctx->session = NULL;
    ctx->api = NULL;
    ctx->ks.fd = -1;

    LINF("Cleanup fetching leaving guarded zone");
    CTX_SOCK_UNLOCK(ctx);
    LINF("Cleanup fetched pointers....");

    bool freed = false;

    LDBG("Cleanup setup: REFCNT: %d, FD: %d, API: %p, CTX: %p, Rust session: %p", 
           stcp_ctx_ref_count_is_what(ctx), 
           sockFD, 
           theAPI, 
           ctx, 
           session);

    freed = stcp_ctx_ref_count_put(ctx);

    if (!freed) {
        LDBG("Not last reference, leaving cleanup");
        return;
    }

    int is_rust_session_valid = 0;
    LDBG("Cleanup: RUST Session? %p", session);
    if (atomic_cas(&ctx->destroyed, 0, 1)) {
        is_rust_session_valid = rust_session_is_valid(session);
        LDBG("Cleanup: RUST Session if set, is valid: %d", is_rust_session_valid);
        if (is_rust_session_valid) {
            LDBG("Cleanup: RUST Session %p...", session);
            rust_exported_session_destroy(session);
            LDBG("Cleanup: RUST Session cleaned..");
        }
    }

    /* vapauta dns info */
    if ( ctx->dns_resolved != NULL ) {
        LDBG("Cleanup: Frreeing DNS information...");
        stcp_dns_free(ctx->dns_resolved);
    }

    /* Sulje socket */
    LDBG("Cleanup: Socket? %d", sockFD);
    if (sockFD >= 0) {
        LDBG("Cleanup: socket...");
        STCP_CLOSE_FD(sockFD);
    }

#ifdef CONFIG_STCP_DEBUG_POISON
    LDBG("Poisoning memory...");
    /* Debug poison */
    memset(ctx, 0xDE, sizeof(*ctx));
    if (theAPI != NULL) {
        memset(theAPI, 0xAD, sizeof(*theAPI));
    }
#endif
 
    if (freed) {
        LDBG("Freeing API (%p) & CTX (%p)", theAPI, ctx);
        if (theAPI) {
            LDBG("Freeing API...");
            k_free(theAPI);
        }

        LDBG("Freeing CTX...");
        k_free(ctx);
    }
    LDBG("Work exit");
    k_yield();
}

int worker_is_context_scheduled_for_cleanup(struct stcp_ctx * ctx)
{
    LDBG("Context %p marked?", ctx);
    if (!ctx) {
        return -EBADFD;
    }
    LDBG("Context %p marked for cleanup: %lu (refcnt: %lu)",
        ctx, 
        atomic_get(&ctx->closing),
        atomic_get(&ctx->refcnt)
    );
    return atomic_get(&ctx->closing);
}

void worker_set_context_scheduled_for_cleanup(struct stcp_ctx * ctx)
{
    LDBG("Setting context %p marked for cleanup...", ctx);
    if (!ctx) {
        return;
    }
    atomic_set(&ctx->closing, 1);
}

void worker_schedule_cleanup(struct stcp_ctx * ctx) {
    LDBG("Cleanup %p ?", ctx);

    if (!ctx) {
        return;
    }

    if (worker_is_context_scheduled_for_cleanup(ctx)) {
        //LDBG("Already in closing...");
        //stcp_dump_bt();
        return;
    }

    worker_context_init(ctx);

    LDBG("Cleanup for %p scheduling..", ctx);
    atomic_set(&ctx->closing, 1);
    /* Aikatauluta cleanup */
    LDBG("Closing kernel socket %p fd %d", ctx, ctx->ks.fd);
    if (ctx->ks.fd >= 0) {
        STCP_SOCKET_CLOSE(ctx->ks.fd);
        ctx->ks.fd = -1;
    }
    k_work_submit_to_queue(&stcp_work_cleanup_q, &ctx->cleanup_work);
    LDBG("Cleanup for %p scheduled!", ctx);
}
