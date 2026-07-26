# Raspberry Pi UDP carrier benchmark

Generated: 2026-07-26T11:45:43+00:00
Platform: Raspberry Pi
Transport: UDP

## Test status

| Protocol | Cases | Passed | Failed | Pass rate | Errors |
|---|---:|---:|---:|---:|---:|
| Raw UDP | 105 | 105 | 0 | 100.0% | 0 |
| STCP/UDP | 105 | 105 | 0 | 100.0% | 0 |
| TLS/TCP reference | 145 | 145 | 0 | 100.0% | 0 |

## Pairwise median changes

Positive throughput/ops is better; negative latency/CPU/cycles/instructions is better.

### STCP VS TLS

Matched successful cases: 105

- operations_s: +36.38%
- combined_mib_s: +36.38%
- rtt_p50_ms: -17.93%
- rtt_p95_ms: -32.32%
- rtt_p99_ms: -31.44%
- connect_mean_ms: -91.44%
- client_cpu_percent: -43.44%
- server_perf_cycles_per_op: -28.08%
- server_perf_instructions_per_op: -45.61%

### STCP VS TCP

Matched successful cases: 0


### STCP VS UDP

Matched successful cases: 105

- operations_s: -2.38%
- combined_mib_s: -2.38%
- rtt_p50_ms: +0.64%
- rtt_p95_ms: +11.68%
- rtt_p99_ms: +6.56%
- connect_mean_ms: +1526.83%
- client_cpu_percent: -47.31%
- server_perf_cycles_per_op: +21.25%
- server_perf_instructions_per_op: +63.87%

