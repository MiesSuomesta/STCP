Patch based on ORIGINAL 14.10.2026 STCP bundle. Includes 128 B AUTO and socket counters; DO NOT apply prior AUTO/counter patches first.
Linux: SOL_STCP=45 / STCP_SO_AUTO_LEVEL_STATS=5. Zephyr module-v2: STCP_SOL_PROTOCOL=253 / STCP_SO_AUTO_LEVEL_STATS=5.
Both return struct { uint64_t off; uint64_t levels[5]; } (48 bytes). Counters are per socket, not reset on read.
Apply: git apply --check stcpv7-auto-counters-complete.patch && git apply stcpv7-auto-counters-complete.patch
No Rust compilation, Zephyr device test, or Robot JSON collection performed.
