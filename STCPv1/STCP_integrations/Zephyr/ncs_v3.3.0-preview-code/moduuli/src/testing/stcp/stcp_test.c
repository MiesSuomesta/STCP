#include <zephyr/logging/log.h>
#include <zephyr/kernel.h>
#include <stdbool.h>
#include <errno.h>

#include <stcp_api.h>
#include <stcp/stcp_api_internal.h>
#include <stcp/debug.h>

// Tärkeä määritellä ENNEN stcp_testing_bplate.h:ta
#define LOGTAG     "[STCP/Test/STCP only] "
#include "stcp_testing_bplate.h"

static void test_stcp_max_throughput(int fd)
{
    uint8_t buf[512];
    memset(buf, 0xAA, sizeof(buf));

    uint64_t tx = 0;
    uint64_t rx = 0;

    while (1) {

        int ret = stcp_send(fd, buf, sizeof(buf), 0);

        if (ret > 0) {
            tx += ret;
        }

        ret = stcp_recv(fd, buf, sizeof(buf), 0);

        if (ret > 0) {
            rx += ret;
        }
    }
}
