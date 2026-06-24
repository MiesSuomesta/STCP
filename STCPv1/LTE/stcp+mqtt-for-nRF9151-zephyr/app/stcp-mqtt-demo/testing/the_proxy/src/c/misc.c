#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include "misc.h"
#include <time.h>
#include <stdint.h>
#include <linux/time.h>

int64_t stcp_uptime_ms(void)
{
    struct timespec ts;

    clock_gettime(
        CLOCK_MONOTONIC,
        &ts
    );

    return
        ((int64_t)ts.tv_sec * 1000LL) +
        (ts.tv_nsec / 1000000LL);
}

int64_t k_uptime_get(void)
{
    static int64_t first = 0;
    int64_t now = stcp_uptime_ms();
    if (!first) {
        first = now;
    }
    return now - first;
}

int stcp_api_get_errno() {
    return errno;
}

int stcp_transport_wait_until_ready(
    int seconds
) {
    return 0;
}
