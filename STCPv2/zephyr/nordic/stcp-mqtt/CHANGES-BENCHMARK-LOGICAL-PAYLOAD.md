# Benchmark logical payload / physical chunk fix

- Added `--max-chunk` (default 65536 bytes).
- Payloads larger than the device limit are segmented using the maximum physical chunk.
- Result filenames and `payload_bytes` retain the requested Raspberry Pi/x86 matrix payload.
- Added `chunk_bytes` and `device_payload_bytes` to show the actual Zephyr I/O chunk.
- Recalculates `operations` and `operations_s` from the logical payload size.
- Validates the device chunk range as 1..65536.
