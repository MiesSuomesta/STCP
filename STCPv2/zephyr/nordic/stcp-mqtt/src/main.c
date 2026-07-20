#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <modem/nrf_modem_lib.h>
#include <modem/lte_lc.h>

#include "echo_benchmark.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
    int ret;

    LOG_INF("STCP2 transport benchmark starting");

    ret = nrf_modem_lib_init();
    if (ret < 0) {
        LOG_ERR("nrf_modem_lib_init failed: %d", ret);
        return ret;
    }

    LOG_INF("nRF modem library initialized");

    ret = lte_lc_connect();
    if (ret < 0) {
        LOG_ERR("LTE connection failed: %d", ret);
        return ret;
    }

    LOG_INF("LTE connected");

#if defined(CONFIG_STCP_BENCH_AUTORUN)
    ret = echo_benchmark_run();
    if (ret < 0) {
        LOG_ERR("Benchmark failed: %d", ret);
        return ret;
    }
    LOG_INF("Transport test finished successfully");
#endif

#if defined(CONFIG_STCP_BENCH_SHELL)
    LOG_INF("Benchmark shell ready; type: stcp config show");
#endif

    return 0;
}
