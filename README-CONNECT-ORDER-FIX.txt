STCP Zephyr connect ordering fix
================================

Extract at repository root:
  cd ~/STCP
  unzip -o stcp-zephyr-connect-rx-before-rust-fix-20260827.zip

Writes only:
  STCPv2/zephyr/nordic/module-v2/src/stcp_v2_socket.c

Functional change:
  native TCP connect
    -> stcp_v2_rx_start()
    -> stcp_rust_connect()
    -> stcp_rust_start_handshake()
    -> wait_connected()

Rationale:
shared-core connect/handshake traffic must have a running carrier RX worker
before Rust connect is entered.

On stcp_rust_connect() failure the just-started RX worker is stopped before
returning to preserve lifecycle cleanup.

Diagnostic markers:
  ZORDER 1 before_rx_start
  ZORDER 2 after_rx_start
  ZORDER 3 before_rust_connect
  ZORDER 4 after_rust_connect

No STCPv2/kernel changes.
No shared Rust changes.
