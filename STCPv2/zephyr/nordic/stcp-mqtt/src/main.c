#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <modem/nrf_modem_lib.h>
#include <modem/lte_lc.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

int main(void)
{
    int ret;

    LOG_INF("STCP2 MQTT proxy starting");

    ret = nrf_modem_lib_init();
    if (ret < 0) {
        LOG_ERR("nrf_modem_lib_init failed: %d", ret);
        return ret;
    }

    LOG_INF("nRF modem library initialized");

    ret = lte_lc_connect();
    if (ret < 0) {
        LOG_ERR("LTE connect failed: %d", ret);
        return ret;
    }

    LOG_INF("LTE connected");

    /* STCP socket + MQTT tästä eteenpäin */

    return 0;
}