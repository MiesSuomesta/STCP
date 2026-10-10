Fix for already installed static-frame-pool overlay. Extract into zephyr/nordic/module-v2. Changes CONFIG_STCP_BUFFER_POOL_FRAMES default 32 -> 16. Existing session.rs TX window remains 16; this is a different parameter.

If actual transport config explicitly sets CONFIG_STCP_BUFFER_POOL_FRAMES=32, change it to16. For this project build script actual transport config is zephyr/nordic/ethernet.conf, not application/ethernet.conf. Remove stale CONFIG_STCP_BUFFER_POOL_LARGE=1 if testing original two-large-slot default.

Default storage now 16*1152 + 2*8192 = 34816 bytes (34KiB), saving18432 bytes (18KiB). Still significant static RAM; not final budget architecture. No Linux rebuild needed. Reconfigure/rebuild Zephyr, check .config and RAM report, flash FULL first after boot.

Expected socket-start log STCP_POOL frame=0/16 large=0/2 static_bytes=34816 (actual frame occupancy may include live metadata). Explicit Kconfig values override default. Shared pool16 has no reserved RX capacity; TX window16 can fill it. If pool exhaustion occurs, provide first STCP_POOL line with peak/failures and requested allocation size. Do not interpret pool-full as heap fragmentation.
