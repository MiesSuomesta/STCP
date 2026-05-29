#pragma once
#include <zephyr/kernel.h>
#include <zephyr/sys_clock.h>

int stcp_check_if_access_is_ok(void *pFrom);

struct stcp_debug_stack_item {
    uintptr_t lr;
    uint32_t timestamp;
};

struct stcp_debug_stack_list_item {
    void *next;
    struct stcp_debug_stack_item item;
};

struct stcp_debug_info {
    struct k_mutex                       lock;
    uintptr_t                            stack_ptr;
    struct stcp_debug_stack_list_item   *backtrace;
};

void  stcp_debug_info_free(struct stcp_debug_info* DIP);
void  stcp_debug_info_dump(struct stcp_debug_info* DIP);
void* stcp_debug_info_new();
int stcp_debug_info_count_list_items(struct stcp_debug_info* DIP, struct stcp_debug_stack_list_item *list);
struct stcp_debug_info *stcp_debug_info_snapshot(void);
void stcp_debug_dump_stcp_ctx(void *ptr);

#define stcp_get_timestamp_code ((uint32_t)k_cyc_to_ms_floor32(k_cycle_get_32()))

static inline void* stcp_get_current_thread(void)
{
    if (k_is_in_isr()) {
        return (void *)0xDEADBEEF;
    }

    return k_current_get();
}

static inline void* stcp_get_lr(void)
{
    if (k_is_in_isr()) {
        return (void *)0xDEADBEEF;
    }

    return __builtin_return_address(0);
}


#define STCP_GET_TIMESTAMP  stcp_get_timestamp_code
#define STCP_GET_THREAD     stcp_get_current_thread()

#define LOG_CALLING_SPOT_FIRST  0


#ifndef ALLOW_DEBUG_FROM_THIS
#define ALLOW_DEBUG_FROM_THIS 1
#endif


#if LOG_CALLING_SPOT_FIRST 
    #define DO_FIRST_ON_LOG do { SDBG("Following log from tagging"); } while(0)
#else
    #define DO_FIRST_ON_LOG do { } while(0)
#endif

#define DO_ISR_GATED(code) \
    if (!k_is_in_isr()) { code; }


#define THE_DEBUG_MESSAGE_START        "[%u ms // %p] STCP/%s[%s:%d][LR:%p]: "
#define THE_DEBUG_MESSAGE_ARGS(level)  STCP_GET_TIMESTAMP, STCP_GET_THREAD, level,  __FILE__, __LINE__, stcp_get_lr()

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

#define STCP_EXEC_CODE_IF_NOT_IN_IRS_CONTEXT(CODE) \
    DO_ISR_GATED( do { CODE; } while (0); )

#if CONFIG_STCP_DEBUG && ALLOW_DEBUG_FROM_THIS

# define SDBG(msg, ...) \
    STCP_EXEC_CODE_IF_NOT_IN_IRS_CONTEXT( \
        do { \
            printk(SDBG_MESSAGE_START msg "\n", SDBG_MESSAGE_ARGS, ##__VA_ARGS__); \
        } while (0); \
    )

# define LDBG(msg, ...) \
    STCP_EXEC_CODE_IF_NOT_IN_IRS_CONTEXT( \
        do { \
            DO_FIRST_ON_LOG; \
            printk(DBG_MESSAGE_START msg "\n", DBG_MESSAGE_ARGS, ##__VA_ARGS__); \
        } while (0); \
    )

# define LWRN(msg, ...) \
    STCP_EXEC_CODE_IF_NOT_IN_IRS_CONTEXT( \
        do { \
            DO_FIRST_ON_LOG; \
            printk(WRN_MESSAGE_START msg "\n", WRN_MESSAGE_ARGS, ##__VA_ARGS__); \
        } while (0); \
    )

# define LINF(msg, ...) \
    STCP_EXEC_CODE_IF_NOT_IN_IRS_CONTEXT( \
        do { \
            DO_FIRST_ON_LOG; \
            printk(INF_MESSAGE_START msg "\n", INF_MESSAGE_ARGS, ##__VA_ARGS__); \
        } while (0); \
    )

# define LERR(msg, ...)  \
    STCP_EXEC_CODE_IF_NOT_IN_IRS_CONTEXT( \
        do { \
            DO_FIRST_ON_LOG; \
            printk(ERR_MESSAGE_START msg "\n", ERR_MESSAGE_ARGS, ##__VA_ARGS__); \
        } while (0); \
    )

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

#define SKDBG(fd, strVal) \
    do {                                            \
        LDBG("Sending via %d: %s", fd, strVal);                 \
        zsock_send(fd, strVal, sizeof(strVal), 0);  \
    } while(0)

#define GET_OK_NOK_STR(val)     ((val) ? "OK" : "NOK")
#define GET_YES_NO_STR(val)     ((val) ? "YES" : "NO")

// Backtrace
void stcp_dump_bt(void);
