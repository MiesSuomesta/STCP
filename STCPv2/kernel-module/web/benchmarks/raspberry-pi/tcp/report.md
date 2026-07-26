# Raspberry Pi TCP benchmark

Generated: 2026-07-26T16:27:21+00:00
Platform: Raspberry Pi
Transport: TCP

## Test status

| Protocol | Cases | Passed | Failed | Pass rate | Errors |
|---|---:|---:|---:|---:|---:|
| Raw TCP | 105 | 105 | 0 | 100.0% | 0 |
| STCP/TCP | 105 | 105 | 0 | 100.0% | 0 |
| TLS/TCP | 105 | 105 | 0 | 100.0% | 0 |

## Pairwise median changes

Positive throughput/ops is better; negative latency/CPU/cycles/instructions is better.

### STCP VS TLS

Matched successful cases: 105

- operations_s: +71.25%
- combined_mib_s: +71.25%
- rtt_p50_ms: -38.82%
- rtt_p95_ms: -38.30%
- rtt_p99_ms: -36.65%
- connect_mean_ms: -88.48%
- client_cpu_percent: -53.18%
- server_perf_cycles_per_op: -49.56%
- server_perf_instructions_per_op: -65.02%

### STCP VS TCP

Matched successful cases: 105

- operations_s: -40.99%
- combined_mib_s: -40.99%
- rtt_p50_ms: +80.13%
- rtt_p95_ms: +47.99%
- rtt_p99_ms: +46.95%
- connect_mean_ms: +638.14%
- client_cpu_percent: -2.36%
- server_perf_cycles_per_op: +88.78%
- server_perf_instructions_per_op: +140.45%

### STCP VS UDP

Matched successful cases: 0


