#pragma once

#define LOG_CALLING_SPOT_FIRST  0

#if LOG_CALLING_SPOT_FIRST 
    #define DO_FIRST_ON_LOG do { SDBG("Following log from tagging"); } while(0)
#else
    #define DO_FIRST_ON_LOG do { } while(0)
#endif

#if CONFIG_STCP_DEBUG
# define SDBG(msg, ...) \
    printk("STCP[%s:%d][LR:%p]: " msg "\n", __FILE__, __LINE__ , __builtin_return_address(0), ##__VA_ARGS__)

# define LDBG(msg, ...) \
    do { \
        DO_FIRST_ON_LOG; \
        printk("STCP[%s:%d][LR:%p]: " msg "\n", __FILE__, __LINE__, __builtin_return_address(0) , ##__VA_ARGS__ ); \
    } while (0)
# define LWRN(msg, ...) \
    do { \
        DO_FIRST_ON_LOG; \
        printk("STCP[%s:%d][LR:%p]: " msg "\n", __FILE__, __LINE__, __builtin_return_address(0) , ##__VA_ARGS__ ); \
    } while (0)

# define LINF(msg, ...) \
    do { \
        DO_FIRST_ON_LOG; \
        printk("STCP[%s:%d][LR:%p]: " msg "\n", __FILE__, __LINE__, __builtin_return_address(0) , ##__VA_ARGS__ ); \
    } while (0)

# define LERR(msg, ...)  \
    do { \
        DO_FIRST_ON_LOG; \
        printk("STCP[%s:%d][LR:%p]: " msg "\n", __FILE__, __LINE__, __builtin_return_address(0) , ##__VA_ARGS__ ); \
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

#define LDBGBIG(...)  _STCP_DO_BIG_PRINT(LDBG, ##__VA_ARGS__)
#define LWRNBIG(...)  _STCP_DO_BIG_PRINT(LINF, ##__VA_ARGS__)
#define LINFBIG(...)  _STCP_DO_BIG_PRINT(LWRN, ##__VA_ARGS__)
#define LERRBIG(...)  _STCP_DO_BIG_PRINT(LERR, ##__VA_ARGS__)

// MQTT macros...
#define _STCP_DO_MQTT_BIG_PRINT(MACRO, ...) \
    do { \
        MACRO("[STCP/MQTT] .--------------------------------------------------------------->"); \
        MACRO("[STCP/MQTT] | " __VA_ARGS__); \
        MACRO("[STCP/MQTT] '----------------------------------------->"); \
    } while(0)

#define MDBG(msg, ...)  LDBG("[STCP/MQTT] " msg, ##__VA_ARGS__)
#define MWRN(msg, ...)  LWRN("[STCP/MQTT] " msg, ##__VA_ARGS__)
#define MINF(msg, ...)  LINF("[STCP/MQTT] " msg, ##__VA_ARGS__)
#define MERR(msg, ...)  LERR("[STCP/MQTT] " msg, ##__VA_ARGS__)

#define MDBGBIG(...)  _STCP_DO_MQTT_BIG_PRINT(MDBG, ##__VA_ARGS__)
#define MWRNBIG(...)  _STCP_DO_MQTT_BIG_PRINT(MWRN, ##__VA_ARGS__)
#define MINFBIG(...)  _STCP_DO_MQTT_BIG_PRINT(MINF, ##__VA_ARGS__)
#define MERRBIG(...)  _STCP_DO_MQTT_BIG_PRINT(MERR, ##__VA_ARGS__)


#define STCP_LOG_HEX(label, buf, len) \
do { \
    printk("[STCP] %s (%d): ", label, len); \
    for (int i = 0; i < len; i++) \
        printk("%02x ", ((uint8_t*)buf)[i]); \
    printk("\n"); \
} while(0)

