#include <zephyr/kernel.h>
#include <modem/nrf_modem_lib.h>
#include <modem/lte_lc.h>
#include <stcp/debug.h>

#define LOGTAG     "[STCP/Watchdog] "
#define WDBG(msg, ...)  LDBG(LOGTAG msg, ##__VA_ARGS__)
#define WWRN(msg, ...)  LWRN(LOGTAG msg, ##__VA_ARGS__)
#define WINF(msg, ...)  LINF(LOGTAG msg, ##__VA_ARGS__)
#define WERR(msg, ...)  LERR(LOGTAG msg, ##__VA_ARGS__)

#define WDBGBIG(...)  _STCP_DO_CUSTOM_BIG_PRINT(WDBG, LOGTAG, ##__VA_ARGS__)
#define WWRNBIG(...)  _STCP_DO_CUSTOM_BIG_PRINT(WWRN, LOGTAG, ##__VA_ARGS__)
#define WINFBIG(...)  _STCP_DO_CUSTOM_BIG_PRINT(WINF, LOGTAG, ##__VA_ARGS__)
#define WERRBIG(...)  _STCP_DO_CUSTOM_BIG_PRINT(WERR, LOGTAG, ##__VA_ARGS__)

#if CONFIG_STCP_WATCHDOG_ENABLE
static uint32_t last_activity = 0;
#endif

// Simple => if not WD enabled => NOP call => optimized out
void stcp_watchdog_update_activity(void)
{
#if CONFIG_STCP_WATCHDOG_ENABLE
    last_activity = k_uptime_get_32();
#endif
}


#if CONFIG_STCP_WATCHDOG_ENABLE
static void modem_reset(void)
{
    WINFBIG("Hard resetting modem");

    WINF("LTE: Radio offline...");
    lte_lc_offline();
    WINF("MODEM: shutdown...");
    nrf_modem_lib_shutdown();

    k_sleep(K_SECONDS(2));

    WINF("MODEM: Re-Init..");
    nrf_modem_lib_init();
    WINF("LTE: Radio to normal state.");
    lte_lc_normal();

}


static void check_modem_health(void)
{

    int64_t now = k_uptime_get();
    int64_t timeout = CONFIG_STCP_WATCHDOG_MAX_TIMEOUT * 1000LL;

    if ((now - last_activity) > timeout) {

        LWRN("No traffic for %d seconds → reset modem",
             CONFIG_STCP_WATCHDOG_MAX_TIMEOUT);

        modem_reset();

        last_activity = now;
    }
}

//
//  Main thread
//

void modem_watchdog_thread(void *a, void *b, void *c)
{
    LINF("Watchdog init:");
    LINF("  Check interval: %d seconds", CONFIG_STCP_WATCHDOG_CHECK_INTERVAL);
    LINF("  Reset timeout : %d seconds", CONFIG_STCP_WATCHDOG_MAX_TIMEOUT);
    uint32_t interval = CONFIG_STCP_WATCHDOG_CHECK_INTERVAL * 1000;

    while (1) {

        check_modem_health();
        k_msleep(interval);

    }

}

K_THREAD_DEFINE(modem_watchdog_tid,
                2048,
                modem_watchdog_thread,
                NULL, NULL, NULL,
                5, 0, 0);

#endif // CONFIG_STCP_WATCHDOG_ENABLE
