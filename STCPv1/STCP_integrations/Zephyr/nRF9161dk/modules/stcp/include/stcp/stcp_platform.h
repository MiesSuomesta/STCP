#pragma once

typedef void (*stcp_platform_ready_cb_t)(void);

int stcp_platform_init(stcp_platform_ready_cb_t cb);
bool stcp_platform_is_ready(void);
void stcp_platform_mark_ready(void);
int stcp_platform_soft_reset(void);
int stcp_platform_wait_until_platform_ready(int seconds);
