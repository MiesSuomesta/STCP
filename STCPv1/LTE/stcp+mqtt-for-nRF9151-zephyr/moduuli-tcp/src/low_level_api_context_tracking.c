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
#include <stcp/low_level_api_context_tracking.h>


#if STCP_API_CONTEXT_TRACKING
static int g_stcp_api_contexts_alive = 0;
static struct stcp_api_context_item the_bookeeping_list[STCP_API_CONTEXT_TRACKING_LIST_SIZE] = { 0 };

struct stcp_api_context_item * stcp_api_context_list_get_slot_ptr(int slot) {
    if (slot >= STCP_API_CONTEXT_TRACKING_LIST_SIZE) return NULL;
    if (slot < 0) return NULL;
    return &(the_bookeeping_list[slot]);
}

extern char * stcp_debug_find_symbol_name(uint32_t *pFrom);

int stcp_api_context_list_get_free_slot() {
    int i = 0;
    int freeSlot = 0;
    struct stcp_api_context_item *pItem = the_bookeeping_list;

    while (i < STCP_API_CONTEXT_TRACKING_LIST_SIZE) {
        pItem = stcp_api_context_list_get_slot_ptr(i);
        if (pItem == NULL) {
            continue;
        }

        if (pItem->alive == 0) {
            LDBG("Found dead at %d", i);
            return i;
        }

        if (pItem->alive == 2) {

            LDBG("Found ghost at %d", i);
            // Clenaup
            stcp_debug_info_free(pItem->alloc_dip);
            pItem->alloc_dip = NULL;

            stcp_debug_info_free(pItem->dealloc_dip);
            pItem->dealloc_dip = NULL;

            pItem->alive        = 0;
            pItem->ptr          = NULL;
            pItem->first_lr     = NULL;
            pItem->lr           = NULL;

            return i;
        }

        i++;
    }

    return -1;
}

int stcp_api_context_list_get_slot_with_pointer(void* pPtr) {
    int i = 0;
    struct stcp_api_context_item *pItem = the_bookeeping_list;
    LDBG("Searching for %p ...", pPtr);
    while (i < STCP_API_CONTEXT_TRACKING_LIST_SIZE) {
        pItem = stcp_api_context_list_get_slot_ptr(i);
        if (pItem == NULL) {
            continue;
        }

        if (pItem->ptr == pPtr) {
            LDBG("Got slot %d for %p ...", i, pPtr);
            return i;
        }
        i++;
    }

    return -1;
}

int stcp_api_context_list_use_slot(int slot, void* pPtr, void *lr) {
    struct stcp_api_context_item *pItem = NULL;

    pItem = stcp_api_context_list_get_slot_ptr(slot);
    if (pItem == NULL) {
        return -EINVAL;
    }

    if (pItem->alive != 1) {
        pItem->first_lr     = lr;
        pItem->ptr          = pPtr;
        pItem->alloc_dip    = stcp_debug_info_snapshot();
    }

    pItem->alive    = 1;
    pItem->lr       = lr;

    LDBG("SLOT[%d] set alive { ptr:%p, %s (LR: %p) }", 
            slot, 
            pPtr,
            stcp_debug_find_symbol_name(lr),
            lr);
    return 1;
}

int stcp_api_context_list_free_slot(int slot, void *freeLR) {
    struct stcp_api_context_item *pItem = NULL;

    pItem = stcp_api_context_list_get_slot_ptr(slot);
    if (pItem == NULL) {
        return -EINVAL;
    }

    if (pItem->alive != 1) {
        LDBGBIG("Slot %d not alive..", slot);
        return -ENXIO;
    }

    pItem->lr = freeLR;
    pItem->alive = 2; // May be freed
    pItem->dealloc_dip  = stcp_debug_info_snapshot();
    int allocItems      = stcp_debug_info_count_list_items(pItem->alloc_dip,   pItem->alloc_dip->backtrace);
    int deallocItems    = stcp_debug_info_count_list_items(pItem->dealloc_dip, pItem->dealloc_dip->backtrace);

    LDBGBIG("API instance @ %p: Allocated at %s (%p), Deallocated at %s (%p)", 
            pItem->ptr,
            stcp_debug_find_symbol_name(pItem->first_lr),
            pItem->first_lr,
            stcp_debug_find_symbol_name(freeLR),
            freeLR);

    if (allocItems > 0) {
        LDBG("Allocation trace: ");
        stcp_debug_info_dump(pItem->alloc_dip);
    }
    
    if (deallocItems > 0) {
        LDBG("Deallocation trace: ");
        stcp_debug_info_dump(pItem->dealloc_dip);
    }

    // MUST FREE THIS!
    k_free(pItem->ptr);

    LDBG("SLOT[%d] Freed", slot);
    return 1;
}

void stcp_api_context_dump_status() {
    int i = 0;
    struct stcp_api_context_item *pItem = NULL;

    while (i < STCP_API_CONTEXT_TRACKING_LIST_SIZE) {
        pItem = stcp_api_context_list_get_slot_ptr(i);
        if (pItem == NULL) {
            continue;
        }

        if (pItem->alive > 0) {

            if (pItem->alive > 1) {
                LDBGBIG("API instance @ address %p, Allocated at %s (%p), Deallocated at %s (%p)", 
                        pItem->ptr, 
                        stcp_debug_find_symbol_name(pItem->first_lr),
                        pItem->first_lr,
                        stcp_debug_find_symbol_name(pItem->lr),
                        pItem->lr
                    );
            } else {
                LDBGBIG("API instance @ address %p, Allocated at %s (%p), Deallocated: not yet..", 
                        pItem->ptr, 
                        stcp_debug_find_symbol_name(pItem->first_lr),
                        pItem->first_lr
                    );
            }

            struct stcp_api * pAPI = VOID_TO_API(pItem->ptr);
            struct stcp_ctx * pCTX = VOID_TO_CTX(pAPI);
            LDBG("API %p context %p dump: ", pAPI, pCTX);
            stcp_debug_dump_stcp_ctx(pCTX);
            
            LDBG("Allocation trace: ");
            stcp_debug_info_dump(pItem->alloc_dip);
            
            if (pItem->alive > 1) {
                LDBG("Deallocation trace: ");
                stcp_debug_info_dump(pItem->dealloc_dip);
            }
        }
        i++;
    }
}

void stcp_api_context_dump_allocs_in_brief() {
    int i = 0;
    struct stcp_api_context_item *pItem = NULL;
    LDBGBIG("Memory allocations alive, in brief:");
    while (i < STCP_API_CONTEXT_TRACKING_LIST_SIZE) {
        pItem = stcp_api_context_list_get_slot_ptr(i);
        if (pItem == NULL) {
            return;
        }

        if (pItem->alive > 0) {
            if (pItem->alive > 1) {
                LDBG("API instance: Allocated at %s (%p), Deallocated at %s (%p)", 
                        stcp_debug_find_symbol_name(pItem->first_lr),
                        pItem->first_lr, 
                        stcp_debug_find_symbol_name(pItem->lr),
                        pItem->lr);
            } else {
                LDBG("API instance: Allocated at %s (%p), Deallocated: not yet..", 
                        stcp_debug_find_symbol_name(pItem->first_lr),
                        pItem->first_lr
                    );
            }
        }
        i++;
    }
}

int stcp_track_api_context(void *pPtr, void *lr) {
    
    if (pPtr == NULL) {
        return -EINVAL;
    }

    stcp_context_do_assert_check(pPtr);

    int idx = stcp_api_context_list_get_free_slot();

    if (idx < 0) {
        stcp_api_context_dump_allocs_in_brief();
        stcp_api_context_dump_status();
        return -ENOBUFS;
    }

    g_stcp_api_contexts_alive++;
    stcp_api_context_list_use_slot(idx, pPtr, lr);
    stcp_api_context_dump_allocs_in_brief();
    return idx;
}

int stcp_untrack_api_context(void* pPtr, void* freelr) {

    if (pPtr == NULL) {
        return;
    }

    stcp_context_do_assert_check(pPtr);

    int idx = stcp_api_context_list_get_slot_with_pointer(pPtr);
    
    if (idx < 0) {
        LERR("pPtr NOT RECORDED");
        stcp_api_context_dump_allocs_in_brief();
        stcp_api_context_dump_status();
        stcp_dump_bt();
        LDBG("pPtr %p freed off record!", pPtr);
        k_stcp_free(pPtr);
    } else {
        // Tämä hoitaa muistin vapautuksen slotista
        stcp_api_context_list_free_slot(idx, freelr);
    }

    g_stcp_api_contexts_alive++;
    if (g_stcp_api_contexts_alive < 0) {
        LERR("Unbalanced API operations!");
        stcp_api_context_dump_allocs_in_brief();
        stcp_api_context_dump_status();
        stcp_dump_bt();
    }

    return idx;
}

#endif
