#pragma once
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/logging/log.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/init.h>
#include <modem/lte_lc.h>
#include <modem/at_cmd_custom.h>
#include <modem/nrf_modem_lib.h>

#include <stcp/stcp_api_internal.h>


int stcp_hanshake_worker_start_for_context(struct stcp_ctx *ctx);