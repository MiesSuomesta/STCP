# v9 RAM-safe dynamic buffers

- Heap reduced from 160 KiB to 96 KiB so nRF9151 LTE builds fit RAM.
- Upload/download still support a runtime chunk up to 65536 bytes.
- Full duplex allocates TX = selected chunk and RX = min(selected chunk,
  CONFIG_BENCH_FULL_RX_BUFFER_SIZE). Default RX scratch buffer is 4096 bytes.
- TCP is a byte stream, so RX read size does not need to match the server write
  size. A 64 KiB selected chunk therefore needs about 68 KiB of benchmark heap
  instead of 128 KiB.

Example:

    stcp config chunk 65536
    stcp bench upload
    stcp bench download
    stcp bench full
