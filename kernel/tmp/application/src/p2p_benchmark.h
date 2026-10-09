#ifndef STCP_P2P_BENCHMARK_H
#define STCP_P2P_BENCHMARK_H

#include <stdint.h>

struct p2p_bench_config {
    char host[128];
    char port[8];
    uint32_t payload_bytes;
    uint32_t total_bytes;
    uint32_t timeout_ms;
};

struct p2p_bench_result {
    int status;
    uint32_t bytes_tx;
    uint32_t bytes_rx;
    int64_t elapsed_ms;
    uint64_t tx_bps;
    uint64_t rx_bps;
    uint64_t aggregate_bps;
};

/*
 * Native STCP_P2P backend contract.
 *
 * The default implementation in p2p_benchmark.c is weak and returns -ENOSYS.
 * A future stcp-p2p-core Zephyr adapter replaces these with strong symbols.
 *
 * This deliberately does NOT implement a "close enough" libp2p wire protocol.
 * The userspace rust-libp2p/STCP program remains the golden reference until
 * the native STCP_P2P implementation is bit/protocol compatible.
 */
int stcp_p2p_backend_available(void);
int stcp_p2p_backend_ping(const struct p2p_bench_config *cfg,
                          uint32_t count,
                          uint32_t *ok_count,
                          uint64_t *rtt_sum_us,
                          uint64_t *rtt_min_us,
                          uint64_t *rtt_max_us);
int stcp_p2p_backend_upload(const struct p2p_bench_config *cfg,
                            struct p2p_bench_result *result);
int stcp_p2p_backend_download(const struct p2p_bench_config *cfg,
                              struct p2p_bench_result *result);
int stcp_p2p_backend_full(const struct p2p_bench_config *cfg,
                          struct p2p_bench_result *result);

void p2p_bench_config_defaults(struct p2p_bench_config *cfg);
int p2p_bench_backend_available(void);
int p2p_bench_ping(const struct p2p_bench_config *cfg, uint32_t count,
                   uint32_t *ok_count, uint64_t *rtt_sum_us,
                   uint64_t *rtt_min_us, uint64_t *rtt_max_us);
int p2p_bench_upload(const struct p2p_bench_config *cfg,
                     struct p2p_bench_result *result);
int p2p_bench_download(const struct p2p_bench_config *cfg,
                       struct p2p_bench_result *result);
int p2p_bench_full(const struct p2p_bench_config *cfg,
                   struct p2p_bench_result *result);

/* Shared-core presence/status.  Weak defaults allow old cores to build. */
uint32_t p2p_bench_core_abi(void);
int p2p_bench_core_selftest(void);
int p2p_bench_protocol_ready(void);
int p2p_bench_noise_core_ready(void);
int p2p_bench_noise_selftest(void);

#endif
