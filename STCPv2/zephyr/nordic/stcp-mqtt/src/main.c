#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "echo_benchmark.h"
#include "stcp_lte_transport.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void)
{
    int ret;

    LOG_INF("STCPv2 transport benchmark starting");

    ret = stcp_lte_transport_init();
    if (ret < 0) {
        LOG_ERR("LTE transport initialization failed: %d", ret);
        return ret;
    }

    LOG_INF("LTE transport connected and data path ready");

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
