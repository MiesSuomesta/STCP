# Raspberry Pi TCP vs TLS vs STCP benchmark

| Mode | Clients | Payload | Pipe | MiB/s combined | Ops/s | RTT p50 | RTT p95 | RTT p99 | CPU % | Errors |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| STCP | 1 | 1024 | 1 | 4.70 | 2404.07 | 0.40 | 0.52 | 0.64 | 20.6 | 0 |
| TCP | 1 | 1024 | 1 | 6.90 | 3534.45 | 0.25 | 0.39 | 0.45 | 28.7 | 0 |
| TLS | 1 | 1024 | 1 | 3.08 | 1577.72 | 0.60 | 0.76 | 0.86 | 38.9 | 0 |
