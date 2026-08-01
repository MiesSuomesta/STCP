# Benchmark runner serial readiness fix

- Waits for a real `stcp config show` response instead of sleeping one second.
- Retries readiness probes for up to 40 seconds after board reset/serial open.
- Requires an acknowledgement for every configuration command.
- Does not clear delayed serial input immediately before a benchmark case.
- Stores the complete raw serial session in `serial.log` beside result JSON files.
- Keeps the current external W5500 driver unchanged during Ethernet builds.
