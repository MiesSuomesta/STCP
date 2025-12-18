// debug_bridge.c

#include <linux/printk.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/ratelimit.h>
                       
#include <stcp/rust_log_with_printk.h>
#include <stcp/debug.h>

// Aikaväli & Purskauksen määrä
#define CUT_OFF_TIME_IN_SECONDS 5
#define BURST_MESSAGE_COUNT     50

DEFINE_RATELIMIT_STATE(stcp_ratelimit_state,
                       CUT_OFF_TIME_IN_SECONDS * HZ,   /* aikaväli: 5 s */
                       BURST_MESSAGE_COUNT);      /* burst: 50 viestiä / 5 s */

void stcp_rust_log(int level, const char *buf, size_t len)
{
    // Check ratelimitter
    if (! __ratelimit(&stcp_ratelimit_state) ) {
        // Ratelimit hit!
        return ;
    }

    static const char *lvl[] = {
        "??",   // 0
        "ERR",  // 1
        "WRN",  // 2
        "INF",  // 3
        "DBG",  // 4
        "TRC",  // 5
    };

    char tmp[512];
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
    printk(KERN_INFO "stcp/RUST[%s]: %s\n", lvl[level], tmp);
}
EXPORT_SYMBOL_GPL(stcp_rust_log);
