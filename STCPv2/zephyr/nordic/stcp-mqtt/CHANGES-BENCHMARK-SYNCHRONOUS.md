# Benchmark synchronous result fix

- `stcp bench upload`, `download`, and `full` now run to completion before the shell command returns.
- Machine-readable JSON is emitted only after the final benchmark result exists.
- Output order is now: benchmark logs -> multipart JSON -> success/failure line -> shell prompt.
- Early validation, allocation, connect, and request errors are stored in the matching result object.
- Multipart JSON contains a plain JSON object and remains compatible with `scripts/run-infra-benchmarks.py`.
- `scripts/build-ethernet.sh` does not modify or patch the external W5500 driver.
