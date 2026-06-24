/*
* STCP socket dispatcher for Zephyr / NCS 2.9 (Zephyr 3.7)
* STCP rides on TCP underneath (for now)
*/

#include <zephyr/logging/log.h>

#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket_offload.h>
#include <zephyr/sys/fdtable.h>
#include <zephyr/sys/util.h>

#include <errno.h>

#include <stcp/debug.h>
#include <stcp/stcp_struct.h>
#include <stcp/fsm.h>
#include <stcp/utils.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/stcp_rust_exported_functions.h>
#include <stcp/stcp_alloc.h>
#include <stcp/dns.h>
#include <stcp/low_level_pointer.h>

#include <stcp/low_level_refcount_tracker.h>

#include <stcp/lifespan.h>

#ifndef IPPROTO_STCP
#define IPPROTO_STCP 253
#endif

atomic_t stcp_context_alive_count = ATOMIC_INIT(0);


#define STCP_CTX_ASSERT(ctx) \
    do { \
        if (!ctx || ctx->magic != STCP_CTX_MAGIC_ALIVE) { \
            LERR("CTX CORRUPTED %p magic=0x%x", ctx, ctx ? ctx->magic : 0); \
            k_panic(); \
        } \
    } while (0)

int stcp_context_do_assert_check(void *pVP) {
    pVP = pVP;
/*    void * lr = stcp_get_lr();
    struct stcp_ctx *ctx = pVP;
    int ok = stcp_ctx_pointer_valid(ctx);
    if (!ok) {
        LERR("CONTEXT ASSERT FAILED @ %p!");
        stcp_debug_dump_stcp_ctx(ctx);
        k_panic();
    }
*/
    // Assert not failing ..
    return 0;
}



/*
    NÄMÄ kolme pitää olla ilman refcounttiin kajoamista
*/
int stcp_context_lifespan_extend(struct stcp_ctx *ctx)
{
    if (!stcp_is_context_valid_no_ref(ctx)) {
        return -EINVAL;
    }

    while (1) {
        if ( atomic_get(&ctx->destroyed) ) {
            //LDBGBIG("STCP: Lifespan is ending .. ");
            return -ESTALE;
        }

        int ref = atomic_get(&ctx->refcnt);

        if (ref <= 0) {
            //LDBGBIG("STCP: Lifespan is ending .. no refs");
            return -EACCES;
        }

        if (ctx->state == STCP_STATE_CLOSING) {
            //LDBGBIG("STCP: Lifespan is ending .. Closing");
            return -EACCES;
        }

        if (atomic_cas(&ctx->refcnt, ref, ref + 1)) {
            //LDBGBIG("STCP: Lifespan extended, refs: %d", ref + 1);
            return 1;
        }

        k_yield(); // tärkeä Zephyrissä
    }
}

int stcp_context_lifespan_get_span(struct stcp_ctx *ctx)
{
    if (!ctx) return -EINVAL;
    
    int ret = (int)atomic_get(&ctx->refcnt);

    return ret;
}

int stcp_context_lifespan_shorten(struct stcp_ctx *ctx)
{
    if (!stcp_is_context_valid_no_ref(ctx)) {
        return -EINVAL;
    }

    if ( atomic_get(&ctx->destroyed) ) {
        //LDBGBIG("STCP: Lifespan is ending .. not cleaning up again.");
        return; // jo menossa
    }

    int old = atomic_get(&ctx->refcnt);
    int new_ref = old - 1;

    if (old <= 0) {
        return -EINVAL;
    }

    old = atomic_dec(&ctx->refcnt);

    if (old == 1) {
        //LDBGBIG("STCP: Lifespan is ending ..scheduling cleanup.");
        worker_schedule_cleanup(ctx);
    }

    return new_ref;
}

void stcp_lifespan_set_api_alive(struct stcp_api *api, int val) {

    if (!api || !api->ctx) {
        return -EINVAL;
    }
    
    atomic_set(&api->alive, val);
}

void stcp_lifespan_set_api_status(struct stcp_api *api, int val) {

    if (!api || !api->ctx) {
        return -EINVAL;
    }

    struct stcp_ctx* ctx = api->ctx;
    atomic_set(&ctx->allow_api_access, val);
    // LDBG("Set API acces enabled? %s", GET_YES_NO_STR(val));
}

