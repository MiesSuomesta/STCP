STCPv2 Zephyr RX-ready overlay
================================

Purpose
-------
Remove the first-boot connect/handshake race by adding a dedicated RX startup
barrier. stcp_v2_rx_start() now returns only after the RX thread itself has
started executing. connect_socket() already calls stcp_v2_rx_start() before
stcp_rust_start_handshake(), so no socket.c change is required.

Changed files
-------------
module-v2/include/stcp/stcp_v2_internal.h
module-v2/src/stcp_v2_context.c
module-v2/src/stcp_v2_rx.c

Apply
-----
Extract this archive over the directory that contains module-v2/.
Then rebuild cleanly and flash.

Expected ordering
-----------------
native TCP connect -> RX thread ready -> Rust handshake start -> peer response RX

The startup wait is bounded to 1000 ms. A failure is reported as
"RX thread startup timeout" / -ETIMEDOUT rather than silently racing handshake.
