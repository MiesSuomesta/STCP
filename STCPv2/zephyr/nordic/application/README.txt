STCPv2 Zephyr RX-ready regression fix
=====================================

Restores the RX startup barrier described by the source bundle's
README-RX-READY-OVERLAY.txt but missing from the current module-v2 sources.

Changed files:
  module-v2/include/stcp/stcp_v2_internal.h
  module-v2/src/stcp_v2_context.c
  module-v2/src/stcp_v2_rx.c

Behavior:
  native TCP connect
    -> stcp_v2_rx_start()
    -> wait until RX thread actually executes
    -> Rust handshake start

The wait is bounded to 1000 ms. On failure the adapter returns -ETIMEDOUT and
logs "RX thread startup timeout" instead of racing silently.

No canonical Rust-core files are changed.
