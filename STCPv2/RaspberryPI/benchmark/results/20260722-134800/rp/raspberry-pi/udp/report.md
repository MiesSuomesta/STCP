# Raspberry Pi UDP carrier benchmark

Generated: 2026-07-27T16:02:22+00:00
Platform: Raspberry Pi
Transport: UDP

## Test status

| Protocol | Cases | Passed | Failed | Pass rate | Errors |
|---|---:|---:|---:|---:|---:|
| Raw UDP | 105 | 105 | 0 | 100.0% | 0 |
| STCP/UDP | 105 | 105 | 0 | 100.0% | 0 |
| TLS/TCP reference | 105 | 105 | 0 | 100.0% | 0 |

## Pairwise median changes

Positive throughput/ops is better; negative latency/CPU/cycles/instructions is better.

### STCP VS TLS

Matched successful cases: 105

- operations_s: +27.61%
- combined_mib_s: +27.61%
- rtt_p50_ms: -18.47%
- rtt_p95_ms: -28.02%
- rtt_p99_ms: -24.81%
- connect_mean_ms: -90.81%
- client_cpu_percent: -45.15%
- server_perf_cycles_per_op: -25.90%
- server_perf_instructions_per_op: -40.50%

### STCP VS TCP

Matched successful cases: 0


### STCP VS UDP

Matched successful cases: 105

- operations_s: +8.28%
- combined_mib_s: +8.28%
- rtt_p50_ms: -9.16%
- rtt_p95_ms: +10.50%
- rtt_p99_ms: +9.24%
- connect_mean_ms: +1807.97%
- client_cpu_percent: -47.72%
- server_perf_cycles_per_op: +32.80%
- server_perf_instructions_per_op: +43.35%

