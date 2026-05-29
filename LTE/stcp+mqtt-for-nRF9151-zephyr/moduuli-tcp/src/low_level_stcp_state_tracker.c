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
#include <stcp/low_level_socks.h>

#include <stcp/low_level_stcp_state_tracker.h>

#if STCP_STCP_FSM_TRACKING
K_MUTEX_DEFINE(g_fsm_trace_mutex);

#define MUTEX_LOCK \
    k_mutex_lock(&g_fsm_trace_mutex, K_FOREVER)

#define MUTEX_UNLOCK \
    k_mutex_unlock(&g_fsm_trace_mutex);

static struct stcp_trace_fsm_item* state_list = NULL;

static struct stcp_trace_fsm_item * stcp_trace_fsm_list_get_new_slot_ptr() {
    void * ret = k_stcp_alloc(sizeof(struct stcp_trace_fsm_item));
    if (ret) {
        memset(ret, 0, sizeof(struct stcp_trace_fsm_item));
    } 
    return ret;
}

static struct stcp_trace_fsm_item * stcp_trace_fsm_list_get_last_slot() {
//    MUTEX_LOCK;
        struct stcp_trace_fsm_item * ret = state_list;
        int i = 0;
        if (ret) {
            while (ret->next != NULL) {
                ret = ret->next;
                i++;
            }
        }
//    MUTEX_UNLOCK;
    LDBG("STCP: FSM trace has %d slots, last at %p", i, ret);
    return ret;
}

static struct stcp_trace_fsm_item *stcp_trace_fsm_list_get_free_slot() {
    struct stcp_trace_fsm_item *pLastItem = NULL;
    struct stcp_trace_fsm_item *pItem = NULL;

    //MUTEX_LOCK;

        pLastItem = stcp_trace_fsm_list_get_last_slot();
        pItem = stcp_trace_fsm_list_get_new_slot_ptr();

        if (pLastItem) {
            pLastItem->next = pItem;
        } else {
            state_list = pItem;
        }

        if (pItem) {
            pItem->prev = pLastItem;
        }

    //MUTEX_UNLOCK;

    return pItem;
}


static struct stcp_trace_fsm_item * stcp_trace_fsm_list_get_slot_with_fsm(void *fsm) {
    LDBG("Searching for %p ...", fsm);
  //  MUTEX_LOCK;

        struct stcp_trace_fsm_item * ret = state_list;

        while (ret != NULL) {
            if (ret->fsm == fsm) {
                LDBG("STCP: FSM trace got fsm %p => %p ..", fsm, ret);
                return ret;
            }
            ret = ret->next;
        }

  //  MUTEX_UNLOCK;
    return NULL;
}

static int stcp_trace_fsm_list_use_slot(void *fsm, int state, void *lr) {
    struct stcp_trace_fsm_item *pItem = NULL;

//    MUTEX_LOCK;

        pItem = stcp_trace_fsm_list_get_slot_with_fsm(fsm);

        if (pItem == NULL) {
            pItem = stcp_trace_fsm_list_get_free_slot();
        }

        if (pItem == NULL) {
            LERR("STCP: FSM Trace: NO MANA LEFT!");
            return -ENOBUFS;
        }

        LDBG("STCP: FSM trace setting used fsm %p => %p ..", fsm, pItem);
        pItem->alive        = 1;
        pItem->fsm          = fsm;
        pItem->state        = state;
        pItem->lr           = lr;
        pItem->timestamp    = k_uptime_get_32();
        pItem->thread       = k_current_get();

        LDBGBIG("FSM[%p @ %u / %p] set { fsm: %p, state:%d, lr %p }", 
            pItem,
            pItem->timestamp,
            pItem->thread,
            fsm,
            state,
            lr);

//    MUTEX_UNLOCK;
    return 1;
}

static int stcp_trace_fsm_list_free_slot(struct stcp_trace_fsm_item *pItemToFree) {
    struct stcp_trace_fsm_item *pItemPrevOfFreed = NULL;
    struct stcp_trace_fsm_item *pItemNextOfFreed = NULL;
    MUTEX_LOCK;

        if (!pItemToFree) {
            MUTEX_UNLOCK;
            return -1;
        }

        pItemPrevOfFreed = pItemToFree->prev;
        pItemNextOfFreed = pItemToFree->next;

        // Re chaining
        if (pItemPrevOfFreed) {
            pItemPrevOfFreed->next = pItemNextOfFreed;
        }

        if (pItemNextOfFreed) {
            pItemNextOfFreed->prev = pItemPrevOfFreed;
        }

        if (state_list == pItemToFree) {
            state_list = pItemToFree->next;
        }

        // Nullataan pointterit
        pItemToFree->next = NULL;
        pItemToFree->prev = NULL;

        k_stcp_free(pItemToFree);

    MUTEX_UNLOCK;

    return 1;
}

void stcp_trace_fsm_dump_status() {

    MUTEX_LOCK;

        struct stcp_trace_fsm_item * ret = state_list;
        int i = 0;

        while (ret != NULL) {
            LDBG("SLOT[%d @ %u / %p] set { alive: %d, fsm: %p, state:%d, lr %p }", 
                i,
                ret->timestamp,
                ret->thread,
                ret->alive,
                ret->fsm,
                ret->state,
                ret->lr);

            i++;
            ret = ret->next;
        }
    MUTEX_UNLOCK;

}

int stcp_trace_fsm_set_state(struct stcp_fsm *fsm, stcp_fsm_state_t state) {
    void *lr = __builtin_return_address(0);
    int rc = 0;
    MUTEX_LOCK;

        rc = stcp_trace_fsm_list_use_slot(fsm, (int)state, lr);

    MUTEX_UNLOCK;

#if STCP_SOCKET_TRACKING_VERBOSE
    stcp_trace_fsm_dump_status();
#endif
    return state;
}

#endif
