#ifndef STCP_BENCH_DIAG_H
#define STCP_BENCH_DIAG_H
#include <stdint.h>
#include <zephyr/sys/printk.h>
struct stcp_bench_diag {
    uint64_t send_attempts, send_busy, send_errors, send_total_us, send_max_us;
    uint64_t recv_calls, rx_packets, rx_bytes, rx_errors, rx_total_us, rx_max_us;
    uint64_t rx_drops; /* Unknown unless connected to a driver counter; currently 0. */
};
static inline void stcp_bench_diag_print(const char *transport, const char *direction,
                                         const struct stcp_bench_diag *d)
{
    printk("BENCH_NET_DIAG transport=%s direction=%s send_attempts=%llu send_busy=%llu "
           "send_errors=%llu send_total_us=%llu send_max_us=%llu "
           "recv_calls=%llu rx_packets=%llu rx_bytes=%llu rx_errors=%llu "
           "rx_total_us=%llu rx_max_us=%llu rx_drops=%llu\n",
           transport, direction,
           (unsigned long long)d->send_attempts,
           (unsigned long long)d->send_busy,
           (unsigned long long)d->send_errors,
           (unsigned long long)d->send_total_us,
           (unsigned long long)d->send_max_us,
           (unsigned long long)d->recv_calls,
           (unsigned long long)d->rx_packets,
           (unsigned long long)d->rx_bytes,
           (unsigned long long)d->rx_errors,
           (unsigned long long)d->rx_total_us,
           (unsigned long long)d->rx_max_us,
           (unsigned long long)d->rx_drops);
}
#endif
