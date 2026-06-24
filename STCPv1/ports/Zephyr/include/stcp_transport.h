#pragma once
#include <stddef.h>
#include <stdint.h>

int stcp_transport_set_connected_fd(int fd);
int stcp_transport_init(void);
int stcp_transport_connect(void);
int stcp_transport_wait_until_ready(int seconds);
int stcp_transport_soft_reset(void * vpCtx);

int stcp_lte_is_registered(void);
int stcp_pdn_is_active();
int stcp_is_reset_requested();
int stcp_poll_fd_changes(int fd, int timeout, int events);
int stcp_update_cell_event_wait_until_seen_or_secs_passed(int seconds);
int stcp_pdn_wait_until_active_or_secs_passed(int seconds);


int stcp_transport_send(struct stcp_ctx *ctx, const uint8_t *buf, size_t len);
int stcp_transport_recv(struct stcp_ctx *ctx, uint8_t *buf, size_t maxlen);
void stcp_transport_close(void *vpCtx);
