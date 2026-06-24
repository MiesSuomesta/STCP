#pragma once
#include <zephyr/kernel.h>

#define STCP_GET_TIMESTAMP     ((uint32_t)k_uptime_get_32())


#define LOG_CALLING_SPOT_FIRST  1

#if LOG_CALLING_SPOT_FIRST 
    #define DO_FIRST_ON_LOG do { SDBG("Following log from tagging"); } while(0)
#else
    #define DO_FIRST_ON_LOG do { } while(0)
#endif

#define THE_DEBUG_MESSAGE_START        "[%u ms] STCP/%s[%s:%d][LR:%p]: "
#define THE_DEBUG_MESSAGE_ARGS(level)  STCP_GET_TIMESTAMP, level,  __FILE__, __LINE__, __builtin_return_address(0)

#define DBG_MESSAGE_START THE_DEBUG_MESSAGE_START
#define DBG_MESSAGE_ARGS  THE_DEBUG_MESSAGE_ARGS("DBG")

#define WRN_MESSAGE_START THE_DEBUG_MESSAGE_START
#define WRN_MESSAGE_ARGS  THE_DEBUG_MESSAGE_ARGS("WRN")

#define INF_MESSAGE_START THE_DEBUG_MESSAGE_START
#define INF_MESSAGE_ARGS  THE_DEBUG_MESSAGE_ARGS("INF")

#define ERR_MESSAGE_START THE_DEBUG_MESSAGE_START
#define ERR_MESSAGE_ARGS  THE_DEBUG_MESSAGE_ARGS("ERR")

#define SDBG_MESSAGE_START THE_DEBUG_MESSAGE_START
#define SDBG_MESSAGE_ARGS  THE_DEBUG_MESSAGE_ARGS("DBG")

#if CONFIG_STCP_DEBUG

# define SDBG(msg, ...) \
    do { \
        printk(SDBG_MESSAGE_START msg "\n", SDBG_MESSAGE_ARGS, ##__VA_ARGS__); \
    } while (0)

# define LDBG(msg, ...) \
    do { \
        DO_FIRST_ON_LOG; \
        printk(DBG_MESSAGE_START msg "\n", DBG_MESSAGE_ARGS, ##__VA_ARGS__); \
    } while (0)

# define LWRN(msg, ...) \
    do { \
        DO_FIRST_ON_LOG; \
        printk(WRN_MESSAGE_START msg "\n", WRN_MESSAGE_ARGS, ##__VA_ARGS__); \
    } while (0)

# define LINF(msg, ...) \
    do { \
        DO_FIRST_ON_LOG; \
        printk(INF_MESSAGE_START msg "\n", INF_MESSAGE_ARGS, ##__VA_ARGS__); \
    } while (0)

# define LERR(msg, ...)  \
    do { \
        DO_FIRST_ON_LOG; \
        printk(ERR_MESSAGE_START msg "\n", ERR_MESSAGE_ARGS, ##__VA_ARGS__); \
    } while (0)

#else
# define SDBG(msg, ...)  do { } while(0)
# define LDBG(msg, ...)  do { } while(0)
# define LWRN(msg, ...)  do { } while(0)
# define LINF(msg, ...)  do { } while(0)
# define LERR(msg, ...)  do { } while(0)
#endif

#define STCP_CLOSE_FD(theFD)            \
    do {                                \
        LERR("Closing fd: %d", theFD);  \
        zsock_close(theFD);             \
        LERR("Closed: %d", theFD);      \
    } while(0)

#define STCP_CONTEXT_GUARD_WITH_RET(theCtx, invalidRet) \
    do {                                                \
        if (stcp_is_context_valid(theCtx) != 1) {       \
            LERR("Context %p not valid!", theCtx);      \
            return invalidRet;                          \
        }                                               \
    } while(0)

#define STCP_CONTEXT_GUARD_VOID_RET(theCtx, invalidRet) \
    do {                                                \
        if (stcp_is_context_valid(theCtx) != 1) {       \
            LERR("Context %p not valid!", theCtx);      \
            return;                                     \
        }                                               \
    } while(0)

#define _STCP_DO_CUSTOM_BIG_PRINT(MACRO, MSTART, MARGS, MTAG, ...) \
    do { \
        printk(MSTART "%s%s" , MARGS, MTAG, " .--------------------------------------------------------------->\n"); \
        printk(MSTART "%s%s" , MARGS, MTAG, " | ");                                                                  \
        printk(__VA_ARGS__);                                                                                         \
        printk("\n");                                                                                                \
        printk(MSTART "%s%s" , MARGS, MTAG, " '----------------------------------------->\n");                       \
    } while(0)

#define LDBGBIG(...)  _STCP_DO_CUSTOM_BIG_PRINT(LDBG, DBG_MESSAGE_START, DBG_MESSAGE_ARGS, "", ##__VA_ARGS__)
#define LINFBIG(...)  _STCP_DO_CUSTOM_BIG_PRINT(LINF, INF_MESSAGE_START, INF_MESSAGE_ARGS, "", ##__VA_ARGS__)
#define LWRNBIG(...)  _STCP_DO_CUSTOM_BIG_PRINT(LWRN, WRN_MESSAGE_START, WRN_MESSAGE_ARGS, "", ##__VA_ARGS__)
#define LERRBIG(...)  _STCP_DO_CUSTOM_BIG_PRINT(LERR, ERR_MESSAGE_START, ERR_MESSAGE_ARGS, "", ##__VA_ARGS__)

#define MDBG(msg, ...)  LDBG("[STCP/MQTT] " msg, ##__VA_ARGS__)
#define MWRN(msg, ...)  LWRN("[STCP/MQTT] " msg, ##__VA_ARGS__)
#define MINF(msg, ...)  LINF("[STCP/MQTT] " msg, ##__VA_ARGS__)
#define MERR(msg, ...)  LERR("[STCP/MQTT] " msg, ##__VA_ARGS__)

#define MDBGBIG(...)  _STCP_DO_CUSTOM_BIG_PRINT(LDBG, DBG_MESSAGE_START, DBG_MESSAGE_ARGS, "[STCP/MQTT]", ##__VA_ARGS__)
#define MINFBIG(...)  _STCP_DO_CUSTOM_BIG_PRINT(LINF, INF_MESSAGE_START, INF_MESSAGE_ARGS, "[STCP/MQTT]", ##__VA_ARGS__)
#define MWRNBIG(...)  _STCP_DO_CUSTOM_BIG_PRINT(LWRN, WRN_MESSAGE_START, WRN_MESSAGE_ARGS, "[STCP/MQTT]", ##__VA_ARGS__)
#define MERRBIG(...)  _STCP_DO_CUSTOM_BIG_PRINT(LERR, ERR_MESSAGE_START, ERR_MESSAGE_ARGS, "[STCP/MQTT]", ##__VA_ARGS__)


#define STCP_LOG_HEX(label, buf, len) \
do { \
    printk("[STCP] %s (%d): ", label, len); \
    for (int i = 0; i < len; i++) \
        printk("%02x ", ((uint8_t*)buf)[i]); \
    printk("\n"); \
} while(0)

#define STCP_DBG_CTX_FD(ctx) \
    LDBG("CTX %p FD=%d", ctx, (ctx)->ks.fd)

// Backtrace
void stcp_dump_bt(void);
