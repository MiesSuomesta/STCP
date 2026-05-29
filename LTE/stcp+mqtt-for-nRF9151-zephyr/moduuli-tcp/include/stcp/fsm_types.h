#pragma once
#include <zephyr/kernel.h>
#include <stdbool.h>
#include <zephyr/kernel/thread_stack.h>

#include <stcp/stcp_struct.h>
#include <stcp/stcp_api_internal.h>

struct stcp_api;
struct stcp_ctx;

typedef enum stcp_fsm_state_t {
    STCP_FSM_INIT = 0,
    STCP_FSM_WAIT_LTE,
    STCP_FSM_WAIT_PDN,
    STCP_FSM_WAIT_IP,
    STCP_FSM_WAIT_STABLE,
    STCP_FSM_TCP_CONNECT,
    STCP_FSM_TCP_WAIT_CONNECT,
    STCP_FSM_STCP_HANDSHAKE,
    STCP_FSM_RUN,
    STCP_FSM_TCP_RECONNECT,
    STCP_FSM_FATAL
} stcp_fsm_state_t;


struct stcp_fsm {
    struct k_mutex lock;
    stcp_fsm_state_t state;
    stcp_fsm_state_t last_state;
    uint32_t last_progress;
    uint32_t dead_counter;
    uint32_t eagain_counter;
    uint32_t loop_count;
    struct stcp_api *api;
    bool stop;
    uint32_t last_run_done_ok;
    int worker_init_done;
    struct k_work_delayable cleanup_work;

//    size_t thread_stack_size;
};
