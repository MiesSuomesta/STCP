# Benchmark runner boot/readiness fix

- Waits 15 seconds for nRF9151 + TF-M + W5500 boot before sending shell commands.
- Does not send Ctrl-C during boot or readiness probing.
- Sends a clean CR followed by `stcp config show` every two seconds.
- Accepts `Host      :` and `Transport :` as the definitive ready response.
- Reacts to the `Benchmark shell ready` boot log by issuing an immediate probe.
- Firmware, W5500 driver, polling logic and Ethernet settings are unchanged.
