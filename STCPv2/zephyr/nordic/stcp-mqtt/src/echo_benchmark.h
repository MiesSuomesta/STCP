#ifndef STCP_ECHO_BENCHMARK_H
#define STCP_ECHO_BENCHMARK_H

#include <stdint.h>

enum bench_transport {
    BENCH_TRANSPORT_TCP = 0,
    BENCH_TRANSPORT_STCP = 1,
    BENCH_TRANSPORT_TLS = 2,
};

struct bench_config {
    char host[128];
    char port[8];
    uint32_t chunk_size;
    uint32_t total_bytes;
    uint32_t timeout_ms;
    uint32_t report_interval_ms;
    enum bench_transport transport;
};

struct bench_result {
    int status;
    uint32_t bytes_tx;
    uint32_t bytes_rx;
    int64_t elapsed_ms;
    uint64_t tx_bps;
    uint64_t rx_bps;
    uint64_t aggregate_bps;
};

struct bench_summary {
    struct bench_result upload;
    struct bench_result download;
    struct bench_result full;
};

void bench_config_defaults(struct bench_config *cfg);
const char *bench_transport_name(enum bench_transport transport);
int bench_run_upload(const struct bench_config *cfg);
int bench_run_download(const struct bench_config *cfg);
int bench_run_full(const struct bench_config *cfg);
int bench_run_all(const struct bench_config *cfg);
void bench_print_last_summary(const struct bench_config *cfg);
int echo_benchmark_run(void);

#endif
