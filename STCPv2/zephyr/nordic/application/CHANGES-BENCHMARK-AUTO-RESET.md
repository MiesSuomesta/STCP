# Benchmark runner: automatic Nordic reset

- Resets nRF9151 DK with `nrfutil device reset --serial-number ...` before opening `/dev/ttyACM0`.
- Waits for CDC ACM re-enumeration.
- Gives Zephyr/TF-M/W5500 15 seconds of quiet boot time.
- Does not inject Ctrl-C during readiness detection.
- Adds deterministic `case_filename()` used by result JSON output.
- New options: `--serial-number`, `--boot-wait`, and `--skip-reset`.
