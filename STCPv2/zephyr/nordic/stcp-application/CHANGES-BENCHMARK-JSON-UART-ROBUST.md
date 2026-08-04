# Benchmark JSON UART robustness

- Numbered multipart frames: `STCP_BENCH_JSON_PART index/count data`.
- Firmware repeats each complete JSON frame three times.
- Short 48-byte payloads and 2 ms pacing reduce UART logger drops.
- Runner deduplicates mirrored lines by part index.
- Missing parts are recovered from later repeated frames.
- Runner preserves compatibility with the older unnumbered multipart format.
- W5500 driver, Ethernet polling and benchmark data path are unchanged.
