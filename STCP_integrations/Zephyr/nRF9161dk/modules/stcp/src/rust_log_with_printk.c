// debug_bridge.c
#include <zephyr/logging/log.h>
#include "stcp/debug.h"

#include <zephyr/kernel.h>

#include "stcp/debug.h"
#include "stcp/utils.h"

#define LOG_BUFFER_SIZE_BYTES       (1024*4)
#define STCP_DEFAULT_RUST_LOG_LEVEL  1

__used
__noinline
void stcp_rust_log(int level, const uint8_t *buf, uintptr_t len)
{
/*
    if (! stcp_config_debug_enabled() ) {
        return;
    }
*/
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

    if (!buf || !len)
        return;

    if (len >= sizeof(tmp))
        n = sizeof(tmp) - 1;
    else
        n = len;

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

}

__used
void *stcp_rust_log_keep = (void *)&stcp_rust_log;