#pragma once
#include <zephyr/logging/log.h>

#define LOG_CALLING_SPOT_FIRST  0

#if LOG_CALLING_SPOT_FIRST 
    #define DO_FIRST_ON_LOG do { SDBG("Following log from tagging"); } while(0)
#else
    #define DO_FIRST_ON_LOG do { } while(0)
#endif

#if CONFIG_STCP_DEBUG
# define SDBG(msg, ...) \
    LOG_INF("STCP[%s:%d][LR:%p]: " msg, __FILE__, __LINE__ , __builtin_return_address(0), ##__VA_ARGS__)

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

# define LINF(msg, ...) \
    do { \
        DO_FIRST_ON_LOG; \
        LOG_INF("STCP[%s:%d][LR:%p]: " msg, __FILE__, __LINE__, __builtin_return_address(0) , ##__VA_ARGS__ ); \
    } while (0)

# define LERR(msg, ...)  \
    do { \
        DO_FIRST_ON_LOG; \
        LOG_ERR("STCP[%s:%d][LR:%p]: " msg, __FILE__, __LINE__, __builtin_return_address(0) , ##__VA_ARGS__ ); \
    } while (0)
#else
# define SDBG(msg, ...)  do { } while(0)
# define LDBG(msg, ...)  do { } while(0)
# define LWRN(msg, ...)  do { } while(0)
# define LINF(msg, ...)  do { } while(0)
# define LERR(msg, ...)  do { } while(0)
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

#define _STCP_DO_BIG_PRINT(MACRO, ...) \
    do { \
        MACRO(".--------------------------------------------------------------->"); \
        MACRO("| " __VA_ARGS__); \
        MACRO("'----------------------------------------->"); \
    } while(0)

#define LDBGBIG(...)  _STCP_DO_BIG_PRINT(LDBG, __VA_ARGS__)
#define LWRNBIG(...)  _STCP_DO_BIG_PRINT(LINF, __VA_ARGS__)
#define LINFBIG(...)  _STCP_DO_BIG_PRINT(LWRN, __VA_ARGS__)
#define LERRBIG(...)  _STCP_DO_BIG_PRINT(LERR, __VA_ARGS__)

