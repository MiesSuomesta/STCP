#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>

int stcp_exported_rust_ctx_alive_count(void);

void stcp_hexdump_ascii(const char *prefix, const uint8_t *buf, size_t len);

int stcp_is_context_valid(void *vpCtx);
void stcp_sleep_ms(uint32_t ms);
int stcp_get_pending_fd_error(int fd);
int stcp_tcp_timeout_set_to_fd(int fd, int timeout_ms);