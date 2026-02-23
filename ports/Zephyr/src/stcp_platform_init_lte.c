#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <errno.h>
#include <stdbool.h>
#include <zephyr/init.h>
#include <modem/lte_lc.h>
#include <zephyr/kernel.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/socket.h>
#include <zephyr/net/net_ip.h>
#include <zephyr/sys/atomic.h>
#include <modem/nrf_modem_lib.h>

#include <zephyr/logging/log.h>

#include "debug.h"
#include "stcp_platform.h"
#include "stcp_net.h"

LOG_MODULE_DECLARE(stcp_lte_module);

#ifndef STCP_VERSION
#define STCP_VERSION "unknown"
#endif

#ifndef STCP_BUILD_DATE
#define STCP_BUILD_DATE "unknown"
#endif

#ifndef STCP_GIT_SHA
#define STCP_GIT_SHA "unknown"
#endif

typedef void (*stcp_platform_ready_cb_t)(void);

static K_SEM_DEFINE(platform_ready_sem, 0, 1);
static int _stcp_platform_is_ready = 0;
static stcp_platform_ready_cb_t user_platform_ready_callback = NULL;

// tässä myöhemmin: socket connect, stcp_net_set_sock, rng init, jne.
int stcp_platform_init_banner(void)
{
    LOG_INF(".----<[STCP by Paxsudos IT]>------------------------------------------------------------>");
    LOG_INF("|  ✅ STCP Initialised (Version %s), Protocol number %d", STCP_VERSION, IPPROTO_STCP);
    LOG_INF("|  🕓 Build at %s (%s)", STCP_BUILD_DATE, STCP_GIT_SHA);
    LOG_INF("'----------------------------------------------------------------------'");
    return 0;
}

void stcp_platform_mark_ready(void)
{
    if (!_stcp_platform_is_ready) {
        _stcp_platform_is_ready = 1;
        k_sem_give(&platform_ready_sem);
        if (user_platform_ready_callback != NULL) {
            SDBG("Calling user CB for platform ready");
            user_platform_ready_callback();
        }
    }
}

int stcp_platform_init(stcp_platform_ready_cb_t cb) {
    stcp_platform_soft_reset();
    user_platform_ready_callback = cb;
    return 0;
}

int stcp_platform_soft_reset() {
    k_sem_init(&platform_ready_sem, 0, 1);
     _stcp_platform_is_ready = 0;
    return 0;
}

bool stcp_platform_is_ready(void) {
    return _stcp_platform_is_ready == 1;
}

int stcp_platform_wait_until_platform_ready(int seconds) {
    SDBG("LTE: Waiting for platform ready (%d Seconds)...", seconds);
	int semRet = k_sem_take(&platform_ready_sem, K_SECONDS(seconds));
    if (semRet<0) {
        LOG_ERR("STCP: Platform not ready within %d seconds, RC: %d",
            seconds, semRet);
    }
    SDBG("LTE: Plaform OK?: %d", semRet);
    return semRet;
}

