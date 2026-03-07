#pragma once
#include <zephyr/kernel.h>
#include <stdbool.h>
#include <stcp/stcp_struct.h>

int stcp_fsm_wait_until_reached_ip_network_up(struct stcp_fsm *fsm, int timeout);
int stcp_fsm_wait_until_reached_lte_ready(struct stcp_fsm *fsm, int timeout);
int stcp_fsm_wait_until_reached_pdn_ready(struct stcp_fsm *fsm, int timeout);
int stcp_fsm_wait_until_reached_connect_ready(struct stcp_fsm *fsm, int timeout);


int stcp_fsm_wait_until_reached_stcp_init_ready(int timeout_ms);



void stcp_fsm_init(struct stcp_fsm *fsm, struct stcp_ctx* ctx);
void stcp_fsm_start(struct stcp_fsm *fsm);
void stcp_fsm_notify_lte_ready(struct stcp_fsm *fsm);
void stcp_fsm_notify_pdn_ready(struct stcp_fsm *fsm);

void stcp_fsm_reached_ip_network_up(struct stcp_fsm *fsm);
void stcp_fsm_reached_lte_ready(struct stcp_fsm *fsm);
void stcp_fsm_reached_pdn_ready(struct stcp_fsm *fsm);