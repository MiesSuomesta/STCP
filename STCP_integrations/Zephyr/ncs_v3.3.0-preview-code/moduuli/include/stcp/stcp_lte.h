#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stcp/stcp_api_internal.h>

// LTE 
int stcp_lte_do_full_reset(struct stcp_ctx *ctx, int wait);