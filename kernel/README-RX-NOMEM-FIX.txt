STCPv2 Zephyr RX -ENOMEM fix overlay
=====================================

Carries forward the previous client-first handshake + RX-memory fixes.

Changed files:
  module/rust/src/state.rs
  module/rust/src/session.rs
  module/rust/src/byte_queue.rs
  module-v2/src/stcp_v2_rx.c

New fix:
  fill_application_buffer() no longer allocates temporary Vec batches for
  received data/control frames. Frames are processed directly one at a time.

Why:
  The failing log showed a 40-byte packet type 8 (Close) returning -ENOMEM.
  Packet type 8 has zero payload, but the old RX path unconditionally did a
  received_frames.try_reserve(8) before inspecting the packet type. If the
  Zephyr heap was already tight, even Close/Ack/Pong processing could fail.

Also:
  Restores stcp_v2_rx.c to the normal non-RXDIAG implementation so diagnostic
  hexdumps/log flooding no longer distort timing or consume logger resources.

Expected:
  - no RXDIAG flood
  - Close frame no longer returns -ENOMEM
  - lower peak heap use during sustained stream traffic
