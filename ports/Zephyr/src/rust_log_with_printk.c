// debug_bridge.c
#include <zephyr/logging/log.h>
#include "debug.h"
LOG_MODULE_REGISTER(stcp_loggin_with_ldbg, LOG_LEVEL_INF);

#include <zephyr/kernel.h>

#include "debug.h"

#define LOG_BUFFER_SIZE_BYTES   (1024*4)
void stcp_rust_log(int level, const char *buf, size_t len)
{
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
        level = 4;

    // Yksinkertainen printk – ei mitään ihmeellistä
    LOG_INF("RUST[%s]: %s", lvl[level], tmp);

}
