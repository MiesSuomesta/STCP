#pragma once
#include <zephyr/kernel.h>
#include <stdbool.h>
#include <zephyr/kernel/thread_stack.h>

#include <stcp/stcp_struct.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/fsm_types.h>

int stcp_fsm_check_if_access_is_granted(struct stcp_api* pAPI);
int stcp_fsm_wait_until_reached_ip_network_up(struct stcp_fsm *fsm, int timeout);
int stcp_fsm_wait_until_reached_lte_ready(struct stcp_fsm *fsm, int timeout);
int stcp_fsm_wait_until_reached_pdn_ready(struct stcp_fsm *fsm, int timeout);
int stcp_fsm_wait_until_reached_connect_ready(struct stcp_fsm *fsm, int timeout);
int stcp_fsm_wait_until_reached_stcp_init_ready(int timeout_ms);

int stcp_fms_state_change_validation(struct stcp_fsm *fsm, stcp_fsm_state_t newstate);

void stcp_fsm_instance_start(struct stcp_fsm *fsm, struct stcp_api *ctx);
void stcp_fsm_init(struct stcp_fsm *fsm);
void stcp_fsm_notify_lte_ready(struct stcp_fsm *fsm);
void stcp_fsm_notify_pdn_ready(struct stcp_fsm *fsm);

void stcp_fsm_last_run_done_ok_update(struct stcp_fsm *fsm);
void stcp_fsm_reached_ip_network_up(struct stcp_fsm *fsm);
void stcp_fsm_reached_lte_ready(struct stcp_fsm *fsm);
void stcp_fsm_reached_pdn_ready(struct stcp_fsm *fsm);