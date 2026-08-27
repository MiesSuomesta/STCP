Zephyr printk-only connect/RX trace.

Extract at ~/STCP:
  unzip -o stcp-zephyr-printk-connect-rx-trace-20260827.zip

Writes:
  STCPv2/zephyr/nordic/module-v2/src/stcp_v2_socket.c
  STCPv2/zephyr/nordic/module-v2/src/stcp_v2_rx.c

Markers:
  ZP 1 native connect returned
  ZP 2 rust connect returned
  ZP 3 before rx_start
  ZP 4 after rx_start
  ZP 5 before handshake start
  ZP 6 after handshake start
  ZP 7 before wait_connected

  ZR 1 rx_start enter
  ZR 2 before k_thread_create
  ZR 3 after k_thread_create
  ZR 4 thread entry
  ZR 5 ready give
  ZR 6 ready wait returned

No kernel/shared Rust changes. No protocol behavior changes.
