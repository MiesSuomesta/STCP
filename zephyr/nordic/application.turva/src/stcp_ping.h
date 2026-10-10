#ifndef STCP_PING_H_
#define STCP_PING_H_

#include <stddef.h>
#include <zephyr/shell/shell.h>

int stcp_ping_run(const struct shell *shell, size_t argc, char **argv);

#endif
