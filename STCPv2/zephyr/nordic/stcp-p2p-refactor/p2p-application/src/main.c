#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#if defined(CONFIG_ETH_W5500)
#include "ethernet_status.h"
#endif

LOG_MODULE_REGISTER(p2p_main, LOG_LEVEL_INF);

int main(void)
{
    LOG_INF("STCPv2 native P2P application starting");

#if defined(CONFIG_ETH_W5500)
    k_sleep(K_MSEC(1200));
    ethernet_status_log_startup();
#endif

    LOG_INF("P2P shell ready; type: stcp p2p show");
    return 0;
}
