# Benchmark runner reset/DTR/JSON fix

- Resets the nRF9151 target before opening `/dev/ttyACM0`.
- Does not wait for the J-Link UART bridge device node to disappear during target reset.
- Keeps DTR asserted when the runner closes the port; lowering DTR caused the next run to require a physical reset.
- Waits 15 seconds for Zephyr boot before probing the shell.
- Accepts multipart JSON when `BEGIN` was dropped but `PART` and `END` arrived.
- Reports a clear incomplete-JSON error when UART dropped one or more payload parts.
