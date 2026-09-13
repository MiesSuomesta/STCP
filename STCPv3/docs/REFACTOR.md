# Directory refactor

## Old bundle component -> new location

- `linux-kernel-linux-module/` -> `kernel/module/`
- kernel `testing/` -> `tests/kernel/testing/`
- kernel `stress/` -> `tests/kernel/stress/`
- kernel `python/` -> `tests/kernel/python/`
- kernel `benchmark/` -> `tests/kernel/benchmark/`
- `raspberry-benchmark/` -> `tests/benchmark/raspberrypi/`
- `robot-test-results/` -> `tests/robot/`
- `zephyr-nordic-nRF9151-stcp-module/` -> `zephyr/nordic/module/`
- `zephyr-nordic-nRF9151-stcp-application/` -> `zephyr/nordic/application/`
- Zephyr application `benchmark/` -> `tests/benchmark/zephyr/`
- `zephyr-nordic-echo-server/` -> `tests/zephyr/echo-server/`

No SDK files are included.
