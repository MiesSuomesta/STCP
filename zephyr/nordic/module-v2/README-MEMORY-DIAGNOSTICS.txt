Extract into zephyr/nordic/module-v2. Keep existing 16-frame session.rs patches on Zephyr and Linux. Rebuild Rust core, Zephyr application and flash; run FULL first after boot.

Error-only LOG_ERR diagnostics, independent of trace configuration:
STCP_MEM alloc_failed bytes=N: requested Rust allocation size.
event900 a=stage (1 wire queue append, 2 receive parsing), b=input bytes.
event901 a=pending TX frame count, b=TX retained logical bytes.
event902 a=wire RX queue bytes, b=application RX queue bytes.
event903 a=out-of-order frame count, b=retained ciphertext bytes.
Counters are snapshots at failure, not heap statistics or exact allocated capacities. Concurrent TX/RX can change between snapshots. The diagnostics allocate no new buffers.

Corrects prior logging target: CMake builds stcp_rust_platform.c, not stcp_v2_platform.c. No protocol or window changes. Rust/Zephyr compilation unavailable here; verify build on target. Provide first complete STCP_MEM block plus surrounding error lines.
