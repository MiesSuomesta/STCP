#pragma once
#include <stcp/debug.h>
#include <stcp/stcp_struct.h>
#include <stcp/low_level_operations.h>

#define STCP_REF_COUNT_TRACKING_VERBOSE     0

#define STCP_GET_REFCOUNT_FROM_PTR(ctx) &(VOID_TO_CTX(ctx)->refcnt)

#if STCP_REF_COUNT_TRACKING_VERBOSE

#define STCP_REF_COUNT_TRACE(ref, ctx, name) \
    do {                                                    \
        LDBGBIG(                                        \
            "REF %s ctx=%p rc=%d name=%s @ %s:%d",      \
            ref,                                        \
            ctx,                                        \
            ctx ? (int)atomic_get(&ctx->refcnt) : -1,        \
            name,                                       \
            __FILE__,                                   \
            __LINE__                                    \
        );                                              \
    } while (0)

#define STCP_REF_GET_LOG(ctx) \
    LDBG("CTX %p REF++ => %d @ %s",                  \
        VOID_TO_CTX(ctx),                            \
        atomic_get(STCP_GET_REFCOUNT_FROM_PTR(ctx)), \
        __func__                                     \
    )

#define STCP_REF_PUT_LOG(ctx) \
    LDBG("CTX %p REF-- => %d @ %s",                  \
        VOID_TO_CTX(ctx),                            \
        atomic_get(STCP_GET_REFCOUNT_FROM_PTR(ctx)), \
        __func__                                     \
    )
#else
#define STCP_REF_COUNT_TRACE(ref, ctx, name)
#define STCP_REF_GET_LOG(ctx) 
#define STCP_REF_PUT_LOG(ctx) 
#endif //STCP_REF_COUNT_TRACKING_VERBOSE

#if STCP_REF_COUNT_TRACKING 
struct stcp_ref_count_item {
    void *ctx;
    int   alive;
    int   refcnt;
    void *first_operation_lr;
    void *last_operation_lr;
    struct stcp_debug_info *alloc_dip;
    struct stcp_debug_info *dealloc_dip;
};

struct stcp_ref_count_item * stcp_ref_count_list_get_slot_ptr(int slot);
int stcp_ref_count_list_get_free_slot();

int stcp_ref_count_list_get_slot_with_pointer(void* pContext);
int stcp_ref_count_list_use_slot(int slot, void* pContext, void *lr, int refcnt);
int stcp_ref_count_list_free_slot(int slot);
void stcp_ref_count_dump_status();

int stcp_track_ref_count_get(struct stcp_ctx *ctx);
int stcp_track_ref_count_put(struct stcp_ctx *ctx);


#define STCP_REF_COUNT_GET(ctx, name, CODE)          \
    do {                                             \
        STCP_REF_GET_LOG(ctx);                       \
        if (stcp_track_ref_count_get(ctx) < 1)       \
        { CODE; }  \
    } while(0)


#define STCP_REF_COUNT_PUT(ctx, name)                \
    do {                                             \
        STCP_REF_PUT_LOG(ctx);                       \
        stcp_track_ref_count_put(ctx);               \
    } while(0)

#else

#define STCP_REF_COUNT_GET(ctx, name, CODE)                 \
    do {                                                    \
        STCP_REF_COUNT_TRACE("GET BEFORE", ctx, name);      \
        int __rc = stcp_context_lifespan_extend(ctx);       \
        if (__rc < 1)                                       \
        {                                                   \
            STCP_REF_COUNT_TRACE("GET ERROR", ctx, name);   \
            CODE;                                           \
        }                                                   \
        STCP_REF_COUNT_TRACE("GET AFTER", ctx, name);       \
    } while(0)


#define STCP_REF_COUNT_PUT(ctx, name)                       \
    do {                                                    \
        STCP_REF_COUNT_TRACE("PUT BEFORE", ctx, name);      \
        stcp_context_lifespan_shorten(ctx);                 \
        STCP_REF_COUNT_TRACE("PUT AFTER", ctx, name);       \
    } while(0)

#endif

