#ifndef STCP_ETHERNET_STATUS_H
#define STCP_ETHERNET_STATUS_H

#include <zephyr/shell/shell.h>

int ethernet_status_show(const struct shell *shell);
void ethernet_status_log_startup(void);

#endif
