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

#include <stcp/low_level_operations.h>
#include <stcp/low_level_api_context_tracking.h>
#include <stcp/low_level_refcount_tracker.h>

#include <stcp/dns.h>

#define STCP_CLEANUP_WORKER_STACK_SIZE (10*1024)

K_THREAD_STACK_DEFINE(stcp_worker_stack, STCP_CLEANUP_WORKER_STACK_SIZE);
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
    k_work_init_delayable(&ctx->cleanup_work, worker_cleanup_work_handler);
    ctx->worker_init_done = 1;
    LDBG("Init of worker, DONE");
}

void worker_resubmit_work_handler(struct stcp_ctx *ctx, int sleep_ms) {

    STCP_REF_COUNT_GET(ctx, "Cleanup resheduled", return);
    atomic_set(&ctx->cleanup_is_rescheduled, 1);
    atomic_set(&ctx->cleanup_work_owns_ref, 1);

    LDBGBIG("Rescheduling cleanup, sleeping for about %d ms..", sleep_ms);
    sleep_ms_jitter(sleep_ms, sleep_ms/10);
    k_work_reschedule(&ctx->cleanup_work, K_NO_WAIT);
}


void stcp_rust_session_rust_session_thread_init(void *a, void *b, void *c);
extern atomic_t stcp_context_alive_count;

void worker_cleanup_work_handler(struct k_work *work)
{
    int queued_cleanup = 0;
    int cleanup_owns_ref = 0;
    int cleanup_running = 0;
    LDBG("STCP cleanup staring running for %p\n", work);
    LDBG("WORK=%p", work);

    struct k_work_delayable *dwork =
        k_work_delayable_from_work(work);

    LDBG("DWORK=%p", dwork);

    struct stcp_ctx *ctx =
        CONTAINER_OF(dwork, struct stcp_ctx, cleanup_work);

//    int lifespan = stcp_context_lifespan_extend(ctx);
    queued_cleanup = atomic_cas(&ctx->cleanup_is_rescheduled, 1, 0);
    cleanup_running = atomic_cas(&ctx->cleanup_running, 0, 1);
    cleanup_owns_ref = atomic_cas(&ctx->cleanup_work_owns_ref, 1, 0);

    LDBG("CTX=%p", ctx);
    LDBG("&cleanup_work=%p", &ctx->cleanup_work);
    LDBG("&cleanup_is_rescheduled=%d", queued_cleanup);
    LDBG("&cleanup_owns_ref=%d", cleanup_owns_ref);

    LDBG("At worker %p // %p // %p:", work, dwork, ctx);	

    if ((uintptr_t)ctx < 0x20000000) {
        LERR("CTX POINTER INVALID %p\n", ctx);
        LDBG("Cleanup: Clearing the cleanup running flag from context %p", ctx);
        atomic_set(&ctx->cleanup_running, 0);

        LDBG("Cleanup: Clearing the cleanup cleanup_work_owns_ref flag from context %p", ctx);
        atomic_set(&ctx->cleanup_work_owns_ref, 0);
        if (cleanup_owns_ref) {
            STCP_REF_COUNT_PUT(ctx, "Cleanup ownerchip released");
        }

        if (queued_cleanup) {
            STCP_REF_COUNT_PUT(ctx, "Cleanup ref put");
        }
        return;
    }

    if ( ctx->magic == STCP_CTX_MAGIC_POISON ) {
        LWRN("Context header magic is poisoned already");
        LDBG("Cleanup: Clearing the cleanup running flag from context %p", ctx);
        atomic_set(&ctx->cleanup_running, 0);
        LDBG("Cleanup: Clearing the cleanup cleanup_work_owns_ref flag from context %p", ctx);
        atomic_set(&ctx->cleanup_work_owns_ref, 0);
        if (cleanup_owns_ref) {
            STCP_REF_COUNT_PUT(ctx, "Cleanup ownerchip released");
        }

        if (queued_cleanup) {
            STCP_REF_COUNT_PUT(ctx, "Cleanup ref put");
        }
        return;
    }

    if ( ctx->magic_footer == STCP_CTX_MAGIC_POISON ) {
        LWRN("Context footer magic is poisoned already");
        LDBG("Cleanup: Clearing the cleanup running flag from context %p", ctx);
        atomic_set(&ctx->cleanup_running, 0);
        LDBG("Cleanup: Clearing the cleanup cleanup_work_owns_ref flag from context %p", ctx);
        atomic_set(&ctx->cleanup_work_owns_ref, 0);
        if (cleanup_owns_ref) {
            STCP_REF_COUNT_PUT(ctx, "Cleanup ownerchip released");
        }

        if (queued_cleanup) {
            STCP_REF_COUNT_PUT(ctx, "Cleanup ref put");
        }
        return;
    }

    if (ctx->magic != STCP_CTX_MAGIC_ALIVE) {
        LWRN("CTX %p invalid magic at header", ctx);
        LDBG("Cleanup: Clearing the cleanup running flag from context %p", ctx);
        atomic_set(&ctx->cleanup_running, 0);
        LDBG("Cleanup: Clearing the cleanup cleanup_work_owns_ref flag from context %p", ctx);
        atomic_set(&ctx->cleanup_work_owns_ref, 0);
        if (cleanup_owns_ref) {
            STCP_REF_COUNT_PUT(ctx, "Cleanup ownerchip released");
        }

        if (queued_cleanup) {
            STCP_REF_COUNT_PUT(ctx, "Cleanup ref put");
        }
        return;
    }

    if (ctx->magic_footer != STCP_CTX_MAGIC_ALIVE_FOOTER) {
        LWRN("CTX %p invalid magic at footer", ctx);
        LDBG("Cleanup: Clearing the cleanup running flag from context %p", ctx);
        atomic_set(&ctx->cleanup_running, 0);
        LDBG("Cleanup: Clearing the cleanup cleanup_work_owns_ref flag from context %p", ctx);
        atomic_set(&ctx->cleanup_work_owns_ref, 0);
        if (cleanup_owns_ref) {
            STCP_REF_COUNT_PUT(ctx, "Cleanup ownerchip released");
        }

        if (queued_cleanup) {
            STCP_REF_COUNT_PUT(ctx, "Cleanup ref put");
        }
        return;
    }
    
    int owned_refs = 0;
    if (cleanup_owns_ref) {
        owned_refs++;
    }

    if (queued_cleanup) {
        owned_refs++;
    }

    // Ennen mitään: Tarkistetaan ettei olla jo tuhoamassa.
    if (!cleanup_running) {
        LWRN("Cleanup already exists for %p", ctx);
        LDBG("Cleanup: Clearing the cleanup cleanup_work_owns_ref flag from context %p", ctx);
        atomic_set(&ctx->cleanup_work_owns_ref, 0);
        if (cleanup_owns_ref) {
            STCP_REF_COUNT_PUT(ctx, "Cleanup ownerchip released");
        }

        if (queued_cleanup) {
            STCP_REF_COUNT_PUT(ctx, "Cleanup ref put");
        }
        return;
    }

#if 0
    LDBG("Cleanup: Lifespan: %d", lifespan);
    if (lifespan < 1) {
        LWRNBIG("Context %p already has died, nothing to do!", ctx);
        LDBG("Clenaup: Clearing the cleanup running flag from context %p", ctx);
        atomic_set(&ctx->cleanup_running, 0);
        LDBG("Cleanup: Clearing the cleanup cleanup_work_owns_ref flag from context %p", ctx);
        atomic_set(&ctx->cleanup_work_owns_ref, 0);
        if (cleanup_owns_ref) {
            STCP_REF_COUNT_PUT(ctx, "Cleanup ownerchip released");
        }

        if (queued_cleanup) {
            STCP_REF_COUNT_PUT(ctx, "Cleanup ref put");
        }
        return; // ctx jo mennyt → EI KOSKE
    }
#endif

    struct k_mutex* context_lock = &(ctx->lock);
    int api_holds_lock = 0;
    if (k_mutex_lock(context_lock, K_MSEC(50)) == 0) {
        api_holds_lock = 1;
    }
    LDBG("Cleanup: Already locked context: %d", api_holds_lock);
    
    // if not disowned ...
    if ( atomic_get(&ctx->owns) ) {
        // Otetaan 'oma' refcountti pois 
        LDBGBIG("Context %p is disowned from now on..", ctx);
        STCP_REF_COUNT_PUT(ctx, "@ Cleanup: Disown API");
		atomic_set(&ctx->owns, 0);
    } else {
        LWRNBIG("Context %p is already disowned..", ctx);
    }


    // Force API closed => No new refs alloved after... set as Dead and Closed.
    stcp_lifespan_set_api_alive(ctx->api, 0);
    stcp_lifespan_set_api_status(ctx->api, 0);
    
    LDBG("WORK=%p\n", work);
    LDBG("CTX=%p, \n", ctx);
    LDBG("&cleanup_work=%p\n", &ctx->cleanup_work);

    LDBG("STCP cleanup context running for %p\n", ctx);


    /* API CLOSE saattaa pitää lukkoa jo, timeoutti varona... */
    int refCnt = atomic_get(&ctx->refcnt);

    LERR("REFCNT at cleanup of ctx=%p ref=%d > %d", ctx, refCnt, owned_refs);

    if (refCnt > owned_refs) {
        if (api_holds_lock) {
            LDBG("Clenaup: Refcount releasing lock of %p", ctx);
            k_mutex_unlock(context_lock);
        } else {
            LDBG("Clenaup: API does not hold lock for ctx %p", ctx);
        }
        
        LDBG("Cleanup: Clearing the cleanup running flag from context %p", ctx);
        atomic_set(&ctx->cleanup_running, 0);
        LDBG("Cleanup: Clearing the cleanup cleanup_work_owns_ref flag from context %p", ctx);
        atomic_set(&ctx->cleanup_work_owns_ref, 0);
        if (cleanup_owns_ref) {
            STCP_REF_COUNT_PUT(ctx, "Cleanup ownerchip released");
        }


        LDBGBIG("Context %p has %d references alive, not doing anything for now.. rescheduling cleanup..", ctx, refCnt);
        worker_resubmit_work_handler(ctx, 500);
        if (queued_cleanup) {
            STCP_REF_COUNT_PUT(ctx, "Cleanup ref put");
        }
        return;
    }

    LINFBIG("Context %p has no other references but self, starting cleanup..", ctx);

    LINF("Cleanup fetching pointers in guarded zone");

    void *session = ctx->session;
    struct stcp_api *theAPI = ctx->api;
    int sockFD = ctx->ks.fd;
    void *recv_stream_buffer = ctx->rx_stream.buffer;
        
    LDBG("Cleanup setup: REFCNT: %d, RX BUF: %p FD: %d, API: %p, CTX: %p, Rust session: %p", 
           stcp_context_lifespan_get_span(ctx), 
           recv_stream_buffer,
           sockFD, 
           theAPI, 
           ctx, 
           session);

    LDBG("Cleanup: RUST Session? %p", session);
    if (session) {
        
        struct k_sem sema_session;
        k_sem_init(&sema_session, 0, 1);

        LDBG("Cleanup: RUST Session %p...starting heavy cleanup....", session);
        stcp_rust_session_rust_session_thread_init(session, &sema_session, NULL);
        k_sem_take(&sema_session, K_FOREVER);
        LDBG("Cleanup: RUST Session %p...starting heavy cleanup done", session);
    }

    /* Sulje socket */
    if (!ctx->doing_replace) {
        LDBG("Cleanup: Socket? %d", sockFD);
        if (sockFD >= 0) {
            LDBG("Cleanup: socket...");
            STCP_CLOSE_FD(sockFD);
        }
    } else {
        LWRNBIG("Cleanup: Leaving socket alone, doing replace!");
    }

    // CLOSE saattaa pitää lukkoa, joka tässä avataan

    // Ensin poisonit, ennen mitään freetä, MUTTA viimeisenä
    //  => mikään ei kosaha jos reworkki tehdään tms.

#ifdef CONFIG_STCP_DEBUG_POISON
    ctx->magic          = STCP_CTX_MAGIC_POISON;
    ctx->magic_footer   = STCP_CTX_MAGIC_POISON;
#endif
    ctx->session        = NULL;
    ctx->api            = NULL;
    ctx->ks.fd          = -1;

    if (api_holds_lock) {
        LDBG("Releasing lock of %p", ctx);
        k_mutex_unlock(context_lock);
    } else {
        LDBG("API does not hold lock of %p", ctx);

    }

    if (cleanup_owns_ref) {
       STCP_REF_COUNT_PUT(ctx, "Cleanup work ownership released");
       atomic_set(&ctx->cleanup_work_owns_ref, 0);
    }

    STCP_REF_COUNT_PUT(ctx, "Cleanup: Decrease lifespan...");

    if (ctx) {
        atomic_set(&ctx->destroyed, 1);
    }

    LDBG("Cleanup: Clearing the cleanup running flag from context %p", ctx);
    atomic_set(&ctx->cleanup_running, 0);
    LDBG("Cleanup: Clearing the cleanup cleanup_work_owns_ref flag from context %p", ctx);
    atomic_set(&ctx->cleanup_work_owns_ref, 0);

    if (queued_cleanup) {
        STCP_REF_COUNT_PUT(ctx, "Cleanup ref put");
    }

    LDBG("Freeing API (%p)", theAPI);
    if (theAPI) {
        LDBG("Freeing API: %p (FSM: %p)", theAPI, theAPI->fsm);
        // Tämä EI deallokoi, vain pitää kirjaa
        STCP_API_CONTEXT_UNTRACK(theAPI); 

        // FSM tappo
        STCP_MEMORY_DEALLOC(theAPI->fsm);
    
        // Tämä deallokoi ok
        STCP_MEMORY_DEALLOC(theAPI);
    }

    // Lukko ihan viime tinkaan vasta vapautetaan...
    STCP_REF_COUNT_PUT(ctx, "@ FINAL PUT"); 
    STCP_MEMORY_DEALLOC(ctx);
    int old = (int)atomic_dec(&stcp_context_alive_count);

    LDBGBIG("Bye from context cleanup! Have a nice day!");
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
 
    LERR("CLEANUP TRIGGERED? cleanup=%d state=%d fd=%d", 
        (int)atomic_get(&ctx->closing), ctx->state, ctx->ks.fd);
    return atomic_get(&ctx->closing);
}

void worker_set_context_scheduled_for_cleanup(struct stcp_ctx * ctx)
{
    LDBG("Setting context %p marked for cleanup...", ctx);

    if (!ctx) {
        LWRN("No context...");
        return;
    }

    if ( stcp_is_context_valid_no_ref(ctx) ) {
        LERR("Context not dead... doing nothing...");
        stcp_dump_bt();
        return;
    }

    STCP_REF_COUNT_GET(ctx, "Initial cleanup queued", return);
    atomic_set(&ctx->cleanup_work_owns_ref, 1);

    LDBG("Setting up for cleanup...");
    atomic_set(&ctx->closing, 1);
    LDBG("Denying access to API..");
    
    atomic_set(&ctx->allow_api_access, 0);
    
    if (ctx->api) {
        struct stcp_api *api = ctx->api;
        LDBG("Set API as dead..");
        atomic_set(&api->alive, 0);
    }
    stcp_debug_dump_stcp_ctx(ctx);
}

void worker_schedule_cleanup(struct stcp_ctx * ctx) {
    LDBG("Cleanup %p ?", ctx);
    if (!ctx) {
        return;
    }

    if ( stcp_is_context_valid_no_ref(ctx) ) {
        LERR("Context not dead... doing nothing...");
        stcp_dump_bt();
        return;
    }

    if (worker_is_context_scheduled_for_cleanup(ctx)) {
        //LDBG("Already in closing...");
        //stcp_dump_bt();
        return;
    }

    if (!ctx->worker_init_done) {
        worker_context_init(ctx);
        ctx->worker_init_done = 1;
    }

    LDBG("Cleanup for %p scheduling..", ctx);
    worker_set_context_scheduled_for_cleanup(ctx);

    k_work_schedule_for_queue(&stcp_work_cleanup_q,
                            &ctx->cleanup_work,
                            K_NO_WAIT);
    LDBG("Cleanup for %p scheduled!", ctx);
}
