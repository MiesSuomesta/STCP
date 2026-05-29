#pragma once 

#include <stcp/settings.h>

#define STCP_SOME_TRACING_ENABLED   \
    STCP_POINTER_TRACKING || \
    STCP_REF_COUNT_TRACKING || \
    STCP_SOCKET_TRACKING || \
    STCP_API_CONTEXT_TRACKING || \
    STCP_STCP_FSM_TRACKING

// Settings
#define STCP_REF_COUNT_TRACKING_LIST_SIZE           16 // 32
#define STCP_POINTER_TRACKING_LIST_SIZE             16 // 32
#define STCP_SOCKET_FD_LIST_SIZE                    16
#define STCP_API_CONTEXT_TRACKING_LIST_SIZE         16

#include <stcp/stcp_alloc.h>
#include <stcp/fsm.h>
#include <stcp/lifespan.h>
#include <stcp/low_level_pointer.h>
#include <stcp/low_level_refcount_tracker.h>
#include <stcp/low_level_socks.h>
#include <stcp/low_level_api_context_tracking.h>
#include <stcp/low_level_stcp_state_tracker.h>
