# Benchmark JSON emitter fix

- Added `bench_last_result()` accessor for upload/download/full results.
- `stcp bench upload`, `download`, and `full` now emit multipart machine JSON immediately after the benchmark function returns.
- JSON uses `STCP_BENCH_JSON_BEGIN`, `STCP_BENCH_JSON_PART`, and `STCP_BENCH_JSON_END` markers understood by `scripts/run-infra-benchmarks.py`.
- Result status, byte counts, elapsed time, throughput, operation count, and infra metadata are included.
- W5500 driver, polling fallback, SPI settings, and Ethernet data path are unchanged.
