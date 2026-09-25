# STCP benchmark matrix

Generated: 2026-09-20T13:33:05+00:00
Source: `/home/pomo/SDK/v4/robot-results/20260920-155907/audio-stream-tests/latest.json`

## Correctness / echo transaction

> `transaction_us` measures the whole echo transaction. It is **not** a persistent bulk-throughput measurement.

| Transport | Mode | PASS | Median transaction | p95 transaction | Reduction | Saved |
|---|---:|---:|---:|---:|---:|---:|
| tcp | off | 58/58 | 8.11 ms | 18.71 ms | - | - |
| udp | off | 58/58 | 28.30 ms | 45.57 ms | - | - |
| tls-tcp | tls | 58/58 | 178.50 ms | 197.13 ms | - | - |
| stcp-tcp | off | 58/58 | 1402.90 ms | 1411.36 ms | - | - |
| stcp-tcp | auto | 58/58 | 1405.21 ms | 1411.74 ms | 24.99% | 1.77 MiB |
| stcp-udp | off | 58/58 | 33.40 ms | 48.73 ms | - | - |
| stcp-udp | auto | 58/58 | 34.84 ms | 45.17 ms | 24.93% | 1.77 MiB |

## Bulk throughput

_No persistent bulk-throughput benchmark data available yet._
