#ifndef BEN2_METRICS_H
#define BEN2_METRICS_H
#include <stdint.h>
#include <stddef.h>
#define BEN2_VERSION 2u
#define BEN2_METRICS_SIZE 112u
#define BEN2_CARRIER_UNKNOWN 0u
#define BEN2_CARRIER_ETHERNET 1u
#define BEN2_CARRIER_LTE 2u
/* All quantities are raw. Times in microseconds. Bytes exclude control frames.
 * Zero means unavailable for times and stack counters. Connect is measured
 * locally, before the data window. Count actual socket calls, not app chunks. */
struct ben2_metrics {
    uint32_t carrier;
    uint64_t tx_bytes, rx_bytes, tx_us, rx_us;
    uint64_t send_calls, recv_calls, eagain, errors;
    uint64_t stack_used_bytes, stack_free_bytes, connect_us, elapsed_us;
};
static inline void ben2_put32(uint8_t *p, uint32_t v) {
    for (int i=3;i>=0;i--) {p[i]=(uint8_t)v;v>>=8;}
}
static inline void ben2_put64(uint8_t *p,uint64_t v) {
    ben2_put32(p,(uint32_t)(v>>32));ben2_put32(p+4,(uint32_t)v);
}
static inline void ben2_request_encode(uint8_t out[20],uint32_t mode,uint32_t chunk,uint32_t total) {
    ben2_put32(out,0x42454e32);ben2_put32(out+4,BEN2_VERSION);
    ben2_put32(out+8,mode);ben2_put32(out+12,chunk);ben2_put32(out+16,total);
}
static inline void ben2_metrics_encode(uint8_t out[BEN2_METRICS_SIZE],const struct ben2_metrics *m) {
    ben2_put32(out,0x4d545232);ben2_put32(out+4,1);ben2_put32(out+8,BEN2_METRICS_SIZE);ben2_put32(out+12,m->carrier);
    const uint64_t values[12]={m->tx_bytes,m->rx_bytes,m->tx_us,m->rx_us,m->send_calls,m->recv_calls,m->eagain,m->errors,m->stack_used_bytes,m->stack_free_bytes,m->connect_us,m->elapsed_us};
    for (size_t i=0;i<12;i++) ben2_put64(out+16+i*8,values[i]);
}
#endif
