# Zephyr STCP reusable buffer pool

Extract this overlay into `zephyr/nordic/module-v2`. It replaces CMakeLists.txt, Kconfig, the Rust ByteQueue implementation, and three C adapter sources; it adds stcp_buffer_pool.c/header. Keep prior 16-frame session.rs on both Zephyr and Linux. Reconfigure and rebuild Rust + Zephyr, check RAM report, flash, run FULL first after reboot. Linux/server require no new changes.

## Allocation model

Static storage, allocated by the linker before traffic: 32 slots x 1152 bytes for frame storage and crypto scratch; 2 slots x 8192 bytes for carrier RX / larger buffers. Total buffer storage 53248 bytes (52 KiB), plus small bookkeeping. Slots are 16-byte aligned. This is a global pool shared by sockets and directions, not 32 slots per socket. Original TX ownership through Rust Arc is preserved: ACK/teardown releases the final owner, then allocator free returns slot. No retransmission overwrites an owned slot. Freed slots are wiped before publishing for reuse. Spinlock protects allocation/free and counters.

Rust allocations >512 bytes use the pool, including frame Vec and Arc storage; allocations <=512 bytes (metadata and small control frames) retain k_malloc. Carrier RX allocates a pool slot once per RX thread and releases at thread exit. AES-GCM and fallback ChaCha in-place decrypt scratch use pool slots. ByteQueue chunk upper bound is 1152 with actual-count reservation. Frame-sized buffers never silently fall back to k_malloc. Thus this removes frame-buffer heap fragmentation, but it is NOT a fully heap-free protocol core.

Pool exhaustion returns NULL and the existing caller reports ENOMEM; no new receiver-advertised flow control is added. 16-frame windows remain needed. The current implementation bounds buffer requests to 8192 bytes; larger Rust/frame/scratch requests fail instead of allocating from heap. Large application transfers should use small writes, as benchmark does (1024 payload slices). This overlay is for present BEN2 traffic, not proof of compatibility with all previous oversized socket writes or all concurrent sockets. Two large slots may limit socket concurrency depending on carrier RX size.

Configuration: CONFIG_STCP_BUFFER_POOL_FRAMES=32 (range16..64), CONFIG_STCP_BUFFER_POOL_LARGE=2 (range1..8). Do not change defaults until measuring actual slot peaks/RAM. Large pool accounts for 8192-byte RX buffers; smaller RX buffers <=1152 consume frame slots. RAM increase is substantial: inspect link memory report. If RAM overflows, provide report instead of arbitrarily reducing slots.

STCP_POOL output: frame and large currently occupied slots / total, frame/large lifetime peaks, failed pool requests, static bytes. Emitted at socket-start and failed Rust allocation. Small heap failures retain earlier STCP_HEAP diagnostics; heap dump is safely disabled unless SYS_HEAP_INFO and SYS_HEAP_VALIDATE are enabled and LOG_PRINTK disabled. These diagnostics are not throughput measurements.

## Validation

Host C pool compiled with -Wall -Wextra -Werror, AddressSanitizer and UndefinedBehaviorSanitizer. Tested all-slot exhaustion, large-request rejection, slot reuse/zeroing, alignment, foreign pointers, and eight threads x 10000 allocations/frees. Final occupancy zero. LeakSanitizer disabled because sandbox process tracing prevents its operation; pool itself uses only static memory. Rust compiler and NCS SDK unavailable, so full module compilation, RAM fit and hardware FULL test remain pending.

Run host pool test: `sh validation/run-pool-tests.sh`.
