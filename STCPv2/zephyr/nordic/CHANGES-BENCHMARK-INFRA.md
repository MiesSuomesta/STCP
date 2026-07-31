# Benchmark infrastructure integration

- Every shell benchmark emits a single `STCP_BENCH_JSON {...}` line.
- JSON follows the existing STCP result vocabulary: mode, transport, clients,
  payload_bytes, pipeline, elapsed_s, operations, errors, throughput fields,
  latency placeholders, platform and carrier.
- `scripts/run-infra-benchmarks.py` controls the Zephyr shell over serial,
  runs a transport/payload/direction matrix and writes one JSON file per case.
- Results are written under `benchmark/results/zephyr-<timestamp>/` by default.
- A `pipeline-summary.json` and `FAILED-ZEPHYR-CASES.tsv` are generated.
- nRF9151/W5500 is represented as clients=1 and pipeline=1, matching the
  device's current single-socket benchmark implementation.

## Build safety fix

The Ethernet benchmark build no longer modifies `eth_w5500.c`. It preserves the
currently installed known-good W5500 polling/IRQ workaround and only rebuilds
the application and benchmark integration.

## Multipart shell JSON fix

- Long benchmark JSON is emitted as `STCP_BENCH_JSON_BEGIN`, multiple
  `STCP_BENCH_JSON_PART` records, and `STCP_BENCH_JSON_END`.
- Each shell record stays below the Zephyr shell printf-buffer limit.
- The serial host runner reassembles and validates the exact declared byte
  length before parsing JSON.
- Legacy one-line `STCP_BENCH_JSON {...}` records remain supported.
