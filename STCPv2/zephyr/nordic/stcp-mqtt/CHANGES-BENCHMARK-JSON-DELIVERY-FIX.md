# Benchmark JSON delivery fix

- Firmware emits `STCP_BENCH_JSON_BEGIN/PART/END` using immediate `printk()` output after the completed benchmark result is stored.
- JSON delivery no longer depends on the Zephyr shell output queue while Ethernet and benchmark logs are busy.
- Host runner processes every serial line exactly once and reconstructs multipart JSON.
- W5500 driver, polling fallback, SPI and network configuration are unchanged.
