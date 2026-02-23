#include <errno.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <modem/lte_lc.h>
#include <modem/pdn.h>

#include <zephyr/net/socket.h>
#include <zephyr/posix/poll.h>

#include <zephyr/net/net_ip.h>
#include <zephyr/sys/atomic.h>
#include <modem/nrf_modem_lib.h>

#include <zephyr/logging/log.h>
#include "debug.h"

LOG_MODULE_REGISTER(stcp_full_soft_reset, LOG_LEVEL_INF);

void stcp_full_soft_reset(void)
{
    LOG_WRN("STCP: Performing full soft reset");

    /* TCP */
    stcp_tcp_close();

    /* STCP */
    stcp_crypto_reset();
    stcp_state_reset();

    /* DNS */
    dns_resolve_cancel_all();

    /* LTE */
    lte_lc_power_off();
    k_sleep(K_SECONDS(2));

    lte_lc_power_on();
}
