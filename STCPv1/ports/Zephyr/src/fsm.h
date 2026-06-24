#pragma once
#include <zephyr/kernel.h>
#include <stdbool.h>

typedef enum {
    STCP_FSM_INIT = 0,
    STCP_FSM_WAIT_LTE,
    STCP_FSM_WAIT_PDN,
    STCP_FSM_WAIT_IP,
    STCP_FSM_WAIT_STABLE,
    STCP_FSM_TCP_CONNECT,
    STCP_FSM_STCP_HANDSHAKE,
    STCP_FSM_RUN,
    STCP_FSM_TCP_RECONNECT,
    STCP_FSM_FATAL
} stcp_fsm_state_t;

struct stcp_fsm {
    stcp_fsm_state_t state;
    struct k_sem lte_ready;
    struct k_sem pdn_ready;
    struct k_sem ip_ready;
    struct stcp_ctx * ctx;
    bool stop;
};

void stcp_fsm_init(struct stcp_fsm *fsm);
void stcp_fsm_start(struct stcp_fsm *fsm);
void stcp_fsm_notify_lte_ready(struct stcp_fsm *fsm);
void stcp_fsm_notify_pdn_ready(struct stcp_fsm *fsm);

extern struct stcp_fsm theFSM;

void stcp_fsm_reached_ip_network_up(struct stcp_fsm *fsm);
void stcp_fsm_reached_lte_ready(struct stcp_fsm *fsm);
void stcp_fsm_reached_pnd_ready(struct stcp_fsm *fsm);