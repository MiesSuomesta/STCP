# W5500 maximum-throughput work — step 1

This package changes only the first low-risk performance layer while preserving
the proven interrupt-polling fallback.

- W5500 SPI clock: 4 MHz -> 8 MHz (nRF9151 maximum target used here).
- TX SENDOK polling interval: 1 ms -> 100 us.
- RX/IRQ polling interval: 1 ms -> 100 us.
- TX timeout: 50 ms -> 10 ms (100 polls).
- Link status check remains approximately once per second.
- Polling statistics are printed approximately every ten seconds.
- Default benchmark chunk and full-duplex RX buffer: 4 KiB -> 16 KiB.
- Ethernet/SPI logging reduced to INFO/WARN to keep UART logging out of the hot path.
- The patcher now adds the required `w5500_rx()` forward declaration.
- Older STCP W5500 patch revisions are restored from `.stcp-original` before
  applying this revision.

Start testing with TCP upload/download/full separately. Compare against the
previous ~1.2 Mbit/s aggregate result before changing any other buffers or TCP
settings.
