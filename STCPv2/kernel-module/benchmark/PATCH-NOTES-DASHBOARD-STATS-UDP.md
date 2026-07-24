# Dashboard statistics and UDP page update

- UDP dashboard now labels protocols explicitly as Raw UDP, STCP/UDP and TLS/TCP reference.
- Legacy TCP baseline is hidden from UDP pages whenever native `udp-c*.json` results exist.
- Executive summary compares STCP/UDP directly against Raw UDP; TLS/TCP is reference-only.
- Added charts for maximum RSS, operations per client CPU percent, MiB/s per client CPU percent, IPC and cache miss rate.
- Added derived efficiency fields to dashboard JSON and CSV:
  - `operations_per_cpu_percent`
  - `mib_per_cpu_percent`
- Existing perf, IRQ, reliability, raw JSON, CSV and manifest outputs remain available.
