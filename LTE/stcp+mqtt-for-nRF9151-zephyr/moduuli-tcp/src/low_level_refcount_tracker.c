#include <zephyr/kernel.h>
#include <zephyr/random/random.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <zephyr/init.h>
#include <zephyr/sys/heap_listener.h>
#include <zephyr/sys/sys_heap.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/sys/atomic.h>

#include <modem/lte_lc.h>
#include <modem/nrf_modem_lib.h>

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <stcp_api.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_net.h>
#include <stcp/workers.h>
#include <stcp/utils.h>
#include <stcp/stcp_mqtt.h>
#include <stcp/utils.h>
#include <stcp/stcp_operations_zephyr.h>
#include <stcp/debug.h>
#include <stcp/low_level_refcount_tracker.h>

#if STCP_REF_COUNT_TRACKING

static struct k_mutex g_debug_mutex_ref_tracking;
static int g_stcp_ref_count_alive = 0;
static struct stcp_ref_count_item the_bookeeping_list[STCP_REF_COUNT_TRACKING_LIST_SIZE] = { 0 };

extern char * stcp_debug_find_symbol_name(uint32_t *pFrom);

#define TRACER_LOCK(CODE) \
    do {                                                                            \
        LDBG("Refcount tracer locking...");                                         \
        int timeout = k_mutex_lock(&(g_debug_mutex_ref_tracking), K_MSEC(100));     \
        if (timeout != 0) { CODE; };                                                \
        LDBG("Refcount tracer locked...");                                          \
    } while(0)

#define TRACER_UNLOCK \
    do {                                                        \
        LDBG("Refcount tracer unlocking...");                   \
        k_mutex_unlock(&(g_debug_mutex_ref_tracking));          \
        LDBG("Refcount tracer unlocked...");                    \
    } while(0)


void stcp_ref_count_tracing_init() {
    static int init = 0;
    if (init) return;
    LDBGBIG("DEBUG: Refcounter tracer init mutex...");
    k_mutex_init(&g_debug_mutex_ref_tracking);

    init = 1;
}

struct stcp_ref_count_item * stcp_ref_count_list_get_slot_ptr(int slot) {
    if (slot < STCP_REF_COUNT_TRACKING_LIST_SIZE) {;
        if (slot > -1) {
            void *ptr = &(the_bookeeping_list[slot]);
            //LDBG("Returning free slot nr: %d => %p", slot, ptr);
            return ptr;
        }
    }
    return NULL;
}

int stcp_ref_count_list_get_free_slot() {
    int i = 0;
    int freeSlot = -1;
    struct stcp_ref_count_item *pItem = the_bookeeping_list;

    while (i < STCP_REF_COUNT_TRACKING_LIST_SIZE) {
        pItem = stcp_ref_count_list_get_slot_ptr(i);
        if (!pItem) {
            i++;
            continue;
        }

        if (!pItem->alive) {
            //LDBG("Returning free slot %d", i);
            freeSlot = i;
            break;
        }

        if (pItem->alive == 2) {
            //LDBG("Returning ghost slot %d", i);
            freeSlot = i;
            break;
        }
        i++;
    }
    //LDBG("Returning free slot nr: %d", freeSlot);
    return freeSlot;
}


int stcp_ref_count_list_get_slot_with_pointer(void* pContext) {
    int i = 0;
    struct stcp_ref_count_item *pItem = the_bookeeping_list;
    while (i < STCP_REF_COUNT_TRACKING_LIST_SIZE) {
        pItem = stcp_ref_count_list_get_slot_ptr(i);
        if (pItem == NULL) {
            i++;
            continue;
        }

        if (pItem->ctx == pContext) {
            return i;
        }
        i++;
    }

    return -1;
}

int stcp_ref_count_list_use_slot(int slot, void* pContext, void *lr, int refCnt) {
    struct stcp_ref_count_item *pItem = NULL;

    pItem = stcp_ref_count_list_get_slot_ptr(slot);
    if (pItem == NULL) {
        LDBG("Got no slot for %d", slot);
        return -EINVAL;
    }

    pItem->alive                = 1;
    pItem->last_operation_lr    = lr;
    pItem->ctx                  = pContext;
    pItem->refcnt               = refCnt;

    if (pItem->first_operation_lr == NULL) {
        pItem->first_operation_lr   = lr; // Laitto vain tässä
        pItem->alloc_dip            = stcp_debug_info_snapshot();
    }

    LDBG("refreshing last trace backtrace...");
    if (pItem->dealloc_dip) {
        stcp_debug_info_free(pItem->dealloc_dip);
        pItem->dealloc_dip = NULL;
    }

    pItem->dealloc_dip  = stcp_debug_info_snapshot();

    LDBG("SLOT[%d] set alive { ctx:%p, flr %p, clr %p }", slot, pContext, pItem->first_operation_lr, lr);
    if (pItem->dealloc_dip) {
        LDBG("Last backtrace of reference change:");
        stcp_debug_info_dump(pItem->dealloc_dip);
    } else {
        LDBG("Last backtrace NULL!");
    }

    return 1;
}

int stcp_ref_count_list_free_slot(int slot) {
    struct stcp_ref_count_item *pItem = NULL;

    pItem = stcp_ref_count_list_get_slot_ptr(slot);

    if (pItem == NULL) {
        LDBG("Got no slot for %d", slot);
        return -EINVAL;
    }

    if (pItem->alloc_dip) {
        stcp_debug_info_free(pItem->alloc_dip);
    }

    if (pItem->dealloc_dip) {
        stcp_debug_info_free(pItem->dealloc_dip);
    }

    memset(pItem, 0, sizeof(struct stcp_ref_count_item));
    LDBG("SLOT[%d] Cleared..", slot);
    return 1;
}

void stcp_ref_count_dump_status() {
    int i = 0;
    struct stcp_ref_count_item *pItem = NULL;

    while (i < STCP_REF_COUNT_TRACKING_LIST_SIZE) {
        pItem = stcp_ref_count_list_get_slot_ptr(i);
        //LDBG("Got slot %d => %p", i, pItem);
        if (pItem == NULL) {
            LDBG("Got no slot for %d", i);
            i++;
            continue;
        }
        
        //int allocItems      = stcp_debug_info_count_list_items(pItem->alloc_dip,   pItem->alloc_dip->backtrace);
        //int deallocItems    = stcp_debug_info_count_list_items(pItem->dealloc_dip, pItem->dealloc_dip->backtrace);

        LINF("Pointer %p (Alive: %d, refcnt: %d) => First LR: %s (%p) and Last LR: %s (%p)",
            pItem->ctx,
            pItem->alive,
            pItem->refcnt,
            stcp_debug_find_symbol_name(pItem->first_operation_lr),
            pItem->first_operation_lr,
            stcp_debug_find_symbol_name(pItem->last_operation_lr),
            pItem->last_operation_lr
        );

        if (pItem->alloc_dip) {
            LDBG("Refcount monitoring stated:");
            stcp_debug_info_dump(pItem->alloc_dip);
        }

        if (pItem->dealloc_dip) {
            LDBG("Refcount last changed:");
            stcp_debug_info_dump(pItem->dealloc_dip);
            stcp_debug_info_free(pItem->dealloc_dip);
            pItem->dealloc_dip = NULL;
        }

        i++;
    }
}

static int stcp_track_ref_count_common_code(int doGet, void *pContext, void *lr) {
        
    if (pContext == NULL) {
        LDBG("Got no context");
        return 0;
    }

    int idx;
    int new = 0;
    idx = stcp_ref_count_list_get_slot_with_pointer(pContext);

    LDBG("[SLOT %p / %d] Got slot nr: %d", pContext, idx, idx);
    if (idx < 0) {
        idx = stcp_ref_count_list_get_free_slot();
        if (idx >= 0) {
            new = 1;
        }
    }

    LDBG("[SLOT %p / %d] Got (New? %d) final slot nr: %d", pContext, idx, new, idx);
    
    if (idx < 0) {
        LDBG("[SLOT %p / %d] Got no slot fro ctx %p", pContext, idx, pContext);
        return idx;
    }

    struct stcp_ref_count_item *pItem = stcp_ref_count_list_get_slot_ptr(idx);
    LDBG("[SLOT %p / %d] Got slot ptr %p", pContext, idx, pItem);

    if (pItem != NULL) {
        if (new) {
            LDBG("[SLOT %p / %d] New context to track refs for", pContext, idx);
            pItem->refcnt = 1;
        } else {
            LDBG("[SLOT %p / %d] Old context to track refs..", pContext, idx);
            if (doGet) {
                pItem->refcnt++;
                LDBG("[SLOT %p / %d] Refcount++ (%d)", pContext, idx, pItem->refcnt);
            } else {
                pItem->refcnt--;
                LDBG("[SLOT %p / %d] Refcount-- (%d)", pContext, idx, pItem->refcnt);
            }

        }

        stcp_ref_count_list_use_slot(idx, pContext, lr, pItem->refcnt);

        if (pItem->refcnt <= 0) {
            LDBG("[SLOT %p / %d] No references, refcnt: %d... dumping status", pContext, idx, pItem->refcnt);
            pItem->alive = 2;
            stcp_ref_count_dump_status();
            LDBG("[Slot %d] No references... cleaning up slot.", idx);
            stcp_ref_count_list_free_slot(idx);
        }
        return 1;
    }
    return 0;
}

#define DO_PUT 0
#define DO_GET 1

int stcp_track_ref_count(void* pContext, void *lr) {
    LDBG("STCP GET REF..");
    if (pContext == NULL) {
        return -1;
    }

    int rc = stcp_track_ref_count_common_code(DO_GET, pContext, lr);
    return rc;
}

int stcp_untrack_ref_count(void* pContext, void *lr) {
    LDBG("STCP PUT REF..");
    if (pContext == NULL) {
        return -1;
    }

    int rc = stcp_track_ref_count_common_code(DO_PUT, pContext, lr);
    return rc;
}

int stcp_track_ref_count_get(struct stcp_ctx *ctx) {
    void *lr =  __builtin_return_address(0);

    stcp_ref_count_tracing_init();
    
    TRACER_LOCK(return);

        LDBG("[TRACKED SLOT] Tracked GET", ctx);
        int ret = stcp_context_lifespan_extend(ctx);
        int slot = stcp_track_ref_count(ctx, lr);


        if (ret < 0) {
            LDBG("[TRACKED SLOT %d (%p)] Failed to track GET @ %s (%p) => rollback ref", slot, ctx, stcp_debug_find_symbol_name(lr), lr);
            stcp_context_lifespan_shorten(ctx);
            TRACER_UNLOCK;
            return;
        }

        LDBG("[TRACKED SLOT %p / %d // %s (%p)] Tracked GET...", ctx, slot, stcp_debug_find_symbol_name(lr), lr);

    TRACER_UNLOCK;

    return 1;
}

int stcp_track_ref_count_put(struct stcp_ctx *ctx) {
    void *lr =  __builtin_return_address(0);

    stcp_ref_count_tracing_init();

    TRACER_LOCK(return);

        int slot = stcp_ref_count_list_get_slot_with_pointer(ctx);
        LDBG("[UNTRACKED SLOT %p / %d // %s (%p)] Tracked PUT...", ctx, slot, stcp_debug_find_symbol_name(lr), lr);
        stcp_context_lifespan_shorten(ctx);
        stcp_untrack_ref_count(ctx, lr);
        LDBG("[UNTRACKED SLOT %p / %d // %p] Tracked PUT", ctx, slot, lr);
    
    TRACER_UNLOCK;
    return 0;
}

#endif
