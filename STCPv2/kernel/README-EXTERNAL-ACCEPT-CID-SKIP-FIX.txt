STCP external accept RX-first handshake fix
===========================================

Problem:
  External TCP child RX can consume the peer PublicKey and assign a non-zero
  connection id before stcp_accept() reaches stcp_rust_start_handshake().
  Calling start_handshake() again may emit a duplicate server PublicKey
  (observed with Zephyr) or return -EINVAL (fast same-host/RPi race).

Fix:
  For external_tcp children only, if connection_id != 0, skip the redundant
  stcp_rust_start_handshake() and proceed directly to the connected wait.
  Keep the existing -EINVAL/nonzero-cid race guard as a fallback.

Scope:
  kernel/module/src/stcp_ops.c only. Shared Rust and Zephyr sources unchanged.

IMPORTANT:
  If kernel/module/src is protected with chattr +i, remove immutability before
  extracting this overlay, then restore it after build/test and commit.
