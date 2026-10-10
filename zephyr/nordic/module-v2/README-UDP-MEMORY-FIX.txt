Extract into zephyr/nordic/module-v2 (the module directory with Kconfig and CMakeLists.txt). Rebuild Rust core and Zephyr application and flash. Do not extract into application.

UDP send window reduced 256 -> 16 frames to bound retained retransmission data and reserve heap space for RX/ACK parsing. ACK cadence remains eight frames. Failed Rust heap allocations now log requested size. No wire changes. Linux module/server unchanged. This is a mitigation for observed ENOMEM, not proof that every allocation issue is resolved. Rust compiler/Zephyr SDK unavailable here; target build and upload verification required.

CMake uses this module's rust directory. Ensure rebuilt libstcp_kernel_core.a includes changed session.rs; remove the module rust/target directory before your normal clean rebuild if build cache is stale.

Single server on 19000:
stcp config transport stcp-udp
stcp config port 19000
stcp bench upload

If ENOMEM persists, provide Rust heap allocation failed: bytes=... and CONFIG_HEAP_MEM_POOL_SIZE from the actual build/zephyr/.config. Do not increase heap without checking available RAM.
