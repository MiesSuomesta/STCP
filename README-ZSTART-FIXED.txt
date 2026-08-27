Fixed Zephyr ZSTART diagnostic overlay.

This replaces only:
  STCPv2/zephyr/nordic/module-v2/src/stcp_v2_rx.c

All stale RXSTAT/RXPROBE/RXDIAG instrumentation was removed.
Only ZSTART diagnostics remain around RX thread startup/ready handling.

No kernel files.
No shared Rust files.
No struct/layout changes.
No protocol behavior changes.
