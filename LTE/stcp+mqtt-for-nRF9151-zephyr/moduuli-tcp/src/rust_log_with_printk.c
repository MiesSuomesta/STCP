// debug_bridge.c
#include <zephyr/logging/log.h>
#include "stcp/debug.h"

#include <zephyr/kernel.h>

#include "stcp/debug.h"
#include "stcp/utils.h"
#include "stcp/sanity.h"

#define LOG_BUFFER_SIZE_BYTES        256
#define STCP_DEFAULT_RUST_LOG_LEVEL  1

static atomic_t stcp_in_log_call = ATOMIC_INIT(0);

__used
__noinline
void stcp_rust_log(int level, const uint8_t *buf, uintptr_t len)
{
#if CONFIG_STCP_DEBUG

    if (!atomic_cas(&stcp_in_log_call, 0, 1)) {
        return;
    }

    atomic_set(&stcp_in_log_call, 1);

    // Check ratelimitter
    static const char *lvl[] = {
        "??",   // 0
        "ERR",  // 1
        "WRN",  // 2
        "INF",  // 3
        "DBG",  // 4
        "TRC",  // 5
    };

    char tmp[LOG_BUFFER_SIZE_BYTES];
    size_t n;

    if (!buf || !len) {
        atomic_set(&stcp_in_log_call, 0);
        return;
    }
    if (len >= sizeof(tmp)) {
        n = sizeof(tmp) - 1;
    } else {
        n = len;
    }
    memcpy(tmp, buf, n);
    tmp[n] = '\0';

    if (level < 0 || level > 5)
        level = STCP_DEFAULT_RUST_LOG_LEVEL;

    // Yksinkertainen printk – ei mitään ihmeellistä
    switch(level) {
        case 1:
        case 2:
        case 3:
        case 4:
            LINF("RUST[%s]: %s", lvl[level],  tmp);
            break;
        default:
            LINF("RUST[%s]: %s", lvl[STCP_DEFAULT_RUST_LOG_LEVEL],  tmp);
            break;
    }

    atomic_set(&stcp_in_log_call, 0);

#endif // CONFIG_STCP_DEBUG
}

__used
void *stcp_rust_log_keep = (void *)&stcp_rust_log;
