#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <stcp/fsm_types.h>
#include <stcp/fsm.h>
#include <stcp/stcp_api.h>
#include <stcp/stcp_struct.h>
#include <stcp/stcp_net.h>
#include <stcp/workers.h>
#include <stcp/utils.h>
#include <stcp/stcp_operations_zephyr.h>

#include <stcp/low_level_operations.h>

#define VOID_TO_STCP_FSM(fsm)           ((struct stcp_fsm *)(fsm))
#define ANY_TO_STCP_FSM_STATE(val)      ((stcp_fsm_state_t)(val))


#if STCP_STCP_FSM_TRACKING

struct stcp_trace_fsm_item {
    void *next;
    void *prev;
    void *fsm;
    int state;
    int alive;
    void *lr;

    uint32_t timestamp;
    k_tid_t thread;
};

void stcp_trace_fsm_dump_status();
int stcp_trace_fsm_set_state(struct stcp_fsm *fsm, stcp_fsm_state_t state);

#define STCP_STCP_FSM_STATE_TRACE(fsm, val)      stcp_trace_fsm_set_state(fsm, val); 

#else
#define STCP_STCP_FSM_STATE_TRACE(fsm, val)
#endif

#define STCP_STCP_FSM_STATE_CHECK(fsm, val) \
        stcp_fms_state_change_validation(VOID_TO_STCP_FSM(fsm), ANY_TO_STCP_FSM_STATE(val))

#define STCP_STCP_FSM_STATE_SET(fsm, val)                       \
    do {                                                        \
        LERRBIG(                                                \
            "[STCP FSM SET] %d -> %d LR=%p",                    \
            VOID_TO_STCP_FSM(fsm)->state,                       \
            val,                                                \
            __builtin_return_address(0)                         \
        );                                                      \
                                                                \
        if (STCP_STCP_FSM_STATE_CHECK(fsm, val) == 1) {         \
            VOID_TO_STCP_FSM(fsm)->state = val;                 \
            STCP_STCP_FSM_STATE_TRACE(fsm, val);                \
        }                                                       \
    } while(0)
