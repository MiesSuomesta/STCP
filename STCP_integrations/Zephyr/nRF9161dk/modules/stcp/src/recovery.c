#include <modem/lte_lc.h>
#include <zephyr/kernel.h>

#include <stcp/debug.h>
#include <stcp/stcp_lte.h>



static int lte_fail_counter;

int stcp_lte_recover(void)
{
    int err;

    LINF("LTE: recover attempt %d", lte_fail_counter);

    err = stcp_reset_lte();

    stcp_utils_sleep_ms_jitter(2000, 400);

    /* 2. yritä reconnect */
    LINF("LTE: Trying to reconnect..");
    err = lte_lc_connect();
    if (!err) {
        LINF("STCP: LTE reconnect started..");
        return 0;
    }

    return err;
}