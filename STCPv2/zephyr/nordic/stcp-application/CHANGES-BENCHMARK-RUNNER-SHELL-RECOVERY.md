# Benchmark runner shell recovery

- Explicitly asserts CDC ACM DTR and deasserts RTS.
- Uses CR only for Zephyr shell command submission.
- Clears stale/partial shell input with Ctrl-C + CR before readiness probes.
- Repeatedly redraws the prompt and sends `stcp config show` without blocking.
- Keeps `/dev/ttyACM0` as the default and does not touch the W5500 driver.
