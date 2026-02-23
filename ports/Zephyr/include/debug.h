#pragma once
#include <zephyr/logging/log.h>
#include "stcp_bridge.h"

#define STCP_FOR_LTE 1

#define LOG_CALLING_SPOT_FIRST  0

#if LOG_CALLING_SPOT_FIRST 
    #define DO_FIRST_ON_LOG do { SDBG("Following log from tagging"); } while(0)
#else
    #define DO_FIRST_ON_LOG do { } while(0)
#endif

#define SDBG(msg, ...) \
    LOG_INF("STCP[%s:%d][LR:%p]: " msg, __FILE__, __LINE__ , __builtin_return_address(0), ##__VA_ARGS__)

#ifdef STCP_FOR_LTE
# define LDBG(msg, ...) \
    do { \
        DO_FIRST_ON_LOG; \
        LOG_INF("STCP[%s:%d][LR:%p]: " msg, __FILE__, __LINE__, __builtin_return_address(0) , ##__VA_ARGS__ ); \
    } while (0)
# define LWRN(msg, ...) \
    do { \
        DO_FIRST_ON_LOG; \
        LOG_WRN("STCP[%s:%d][LR:%p]: " msg, __FILE__, __LINE__, __builtin_return_address(0) , ##__VA_ARGS__ ); \
    } while (0)

# define LERR(msg, ...)  \
    do { \
        DO_FIRST_ON_LOG; \
        LOG_ERR("STCP[%s:%d][LR:%p]: " msg, __FILE__, __LINE__, __builtin_return_address(0) , ##__VA_ARGS__ ); \
    } while (0)
#else
# define LDBG(msg, ...)  do { } while(0)
# define LWRN(msg, ...)  do { } while(0)
# define LERR(msg, ...)  do { } while(0)
#endif

#ifdef STCP_FOR_BT
# define BDBG(msg, ...)  LOG_INF("STCP[Bluetooth]: " msg, ##__VA_ARGS__)
# define BWRN(msg, ...)  LOG_WRN("STCP[Bluetooth]: " msg, ##__VA_ARGS__)
# define BERR(msg, ...)  LOG_ERR("STCP[Bluetooth]: " msg, ##__VA_ARGS__)
#else
# define BDBG(msg, ...)  do { } while(0)
# define BWRN(msg, ...)  do { } while(0)
# define BERR(msg, ...)  do { } while(0)
#endif

#define STCP_CLOSE_FD(theFD) \
    do { \
        LERR("Closing fd: %d", theFD); \
        zsock_close(theFD); \
        LERR("Closed: %d", theFD); \
    } while(0)

#define STCP_CONTEXT_GUARD_WITH_RET(theCtx, invalidRet) \
    do { \
        if (stcp_is_context_valid(theCtx) != 1) { \
            LERR("Context %p not valid!", theCtx); \
            return invalidRet; \
        } \
    } while(0)

#define STCP_CONTEXT_GUARD_VOID_RET(theCtx, invalidRet) \
    do { \
        if (stcp_is_context_valid(theCtx) != 1) { \
            LERR("Context %p not valid!", theCtx); \
            return; \
        } \
    } while(0)
