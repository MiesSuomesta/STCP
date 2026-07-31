# Benchmark infrastructure integration (driver-safe)

- Based directly on the user-provided working source archive.
- W5500 driver is not patched or modified by build-ethernet.sh.
- Adds multipart STCP_BENCH_JSON output to avoid Zephyr shell line truncation.
- Fixes early benchmark failures so JSON status matches the command return code.
- Adds host serial matrix runner with multipart JSON reassembly.
- Keeps Ethernet overlay, polling workaround, SPI settings, and network data path untouched.
