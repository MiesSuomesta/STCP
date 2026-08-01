# Stable W5500 + benchmark infrastructure

- Replaces the unsafe 100 us W5500 patch with a conservative 1 ms fallback.
- RX is not touched before the Zephyr network interface has been initialized.
- RX is not called from the TX polling hot path.
- Keeps 8 MHz SPI and the fixed locally administered MAC address.
- Emits benchmark results as chunked JSON suitable for the host matrix runner.
- Early connect/allocation/request errors update the machine-readable status correctly.
