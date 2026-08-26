STCPv2 Zephyr RXPROBE overlay - 2026-08-26

Purpose:
- Prove that the flashed firmware contains this overlay.
- Prove whether the first native recv()/recvfrom() call returns.

Markers:
  RXPROBE BUILD=20260826-1
  RXPROBE BEFORE_RECV ...
  RXPROBE AFTER_RECV ...

Interpretation:
- BUILD missing: this image is not running.
- BUILD + BEFORE_RECV, but no AFTER_RECV: first native recv() is blocked/stuck.
- AFTER_RECV n>0: data reached Zephyr native socket layer.
- AFTER_RECV n<0: errno shows the immediate socket-layer result.

This overlay retains the existing RX-ready, RXDIAG, and RXSTAT diagnostics.
No Rust-core or wire-protocol changes.
