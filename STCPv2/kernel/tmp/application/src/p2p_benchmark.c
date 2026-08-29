#include <errno.h>
#include <limits.h>
#include <string.h>

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include "p2p_benchmark.h"

__weak int stcp_p2p_backend_available(void)
{
    return 0;
}

__weak int stcp_p2p_backend_ping(const struct p2p_bench_config *cfg,
                                 uint32_t count,
                                 uint32_t *ok_count,
                                 uint64_t *rtt_sum_us,
                                 uint64_t *rtt_min_us,
                                 uint64_t *rtt_max_us)
{
    ARG_UNUSED(cfg);
    ARG_UNUSED(count);
    ARG_UNUSED(ok_count);
    ARG_UNUSED(rtt_sum_us);
    ARG_UNUSED(rtt_min_us);
    ARG_UNUSED(rtt_max_us);
    return -ENOSYS;
}

__weak int stcp_p2p_backend_upload(const struct p2p_bench_config *cfg,
                                   struct p2p_bench_result *result)
{
    ARG_UNUSED(cfg);
    ARG_UNUSED(result);
    return -ENOSYS;
}

__weak int stcp_p2p_backend_download(const struct p2p_bench_config *cfg,
                                     struct p2p_bench_result *result)
{
    ARG_UNUSED(cfg);
    ARG_UNUSED(result);
    return -ENOSYS;
}

__weak int stcp_p2p_backend_full(const struct p2p_bench_config *cfg,
                                 struct p2p_bench_result *result)
{
    ARG_UNUSED(cfg);
    ARG_UNUSED(result);
    return -ENOSYS;
}

void p2p_bench_config_defaults(struct p2p_bench_config *cfg)
{
    memset(cfg, 0, sizeof(*cfg));
    strncpy(cfg->host, CONFIG_P2P_BENCH_HOST, sizeof(cfg->host) - 1U);
    strncpy(cfg->port, CONFIG_P2P_BENCH_PORT, sizeof(cfg->port) - 1U);
    cfg->payload_bytes = CONFIG_P2P_BENCH_PAYLOAD_SIZE;
    cfg->total_bytes = CONFIG_P2P_BENCH_TOTAL_BYTES;
    cfg->timeout_ms = CONFIG_P2P_BENCH_TIMEOUT_MS;
}

int p2p_bench_backend_available(void)
{
    return stcp_p2p_backend_available();
}

int p2p_bench_ping(const struct p2p_bench_config *cfg, uint32_t count,
                   uint32_t *ok_count, uint64_t *rtt_sum_us,
                   uint64_t *rtt_min_us, uint64_t *rtt_max_us)
{
    if (!cfg || !ok_count || !rtt_sum_us || !rtt_min_us || !rtt_max_us ||
        count == 0U) {
        return -EINVAL;
    }

    *ok_count = 0U;
    *rtt_sum_us = 0U;
    *rtt_min_us = UINT64_MAX;
    *rtt_max_us = 0U;

    return stcp_p2p_backend_ping(cfg, count, ok_count, rtt_sum_us,
                                 rtt_min_us, rtt_max_us);
}

static int run_bench(const struct p2p_bench_config *cfg,
                     struct p2p_bench_result *result,
                     int (*fn)(const struct p2p_bench_config *,
                               struct p2p_bench_result *))
{
    if (!cfg || !result || !fn || cfg->payload_bytes == 0U ||
        cfg->total_bytes == 0U) {
        return -EINVAL;
    }

    memset(result, 0, sizeof(*result));
    result->status = -ECANCELED;

    int rc = fn(cfg, result);
    result->status = rc;
    return rc;
}

int p2p_bench_upload(const struct p2p_bench_config *cfg,
                     struct p2p_bench_result *result)
{
    return run_bench(cfg, result, stcp_p2p_backend_upload);
}

int p2p_bench_download(const struct p2p_bench_config *cfg,
                       struct p2p_bench_result *result)
{
    return run_bench(cfg, result, stcp_p2p_backend_download);
}

int p2p_bench_full(const struct p2p_bench_config *cfg,
                   struct p2p_bench_result *result)
{
    return run_bench(cfg, result, stcp_p2p_backend_full);
}

/* The canonical shared Rust core exports these symbols in native P2P phase 1. */
extern uint32_t stcp_p2p_core_abi_version(void);
extern int stcp_p2p_core_selftest(void);
extern int stcp_p2p_core_ready(void);
extern int stcp_p2p_noise_core_ready(void);
extern int stcp_p2p_noise_selftest(void);

uint32_t p2p_bench_core_abi(void)
{
    return stcp_p2p_core_abi_version();
}

int p2p_bench_core_selftest(void)
{
    return stcp_p2p_core_selftest();
}

int p2p_bench_protocol_ready(void)
{
    return stcp_p2p_core_ready();
}

int p2p_bench_noise_core_ready(void)
{
    return stcp_p2p_noise_core_ready();
}

int p2p_bench_noise_selftest(void)
{
    return stcp_p2p_noise_selftest();
}
