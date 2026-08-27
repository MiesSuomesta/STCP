STCP Zephyr RX ready-wait bypass
=================================

Extract from repository root:
  cd ~/STCP
  unzip -o stcp-zephyr-rx-ready-bypass-fix-20260827.zip

Writes only:
  STCPv2/zephyr/nordic/module-v2/src/stcp_v2_rx.c

Behavioral change:
- k_thread_create() remains unchanged.
- RX thread still signals rx_ready.
- stcp_v2_rx_start() no longer blocks on k_sem_take(rx_ready).
- connect can continue immediately after successful thread creation.

Diagnostic:
  RXREADY BYPASS thread_created ...

No STCPv2/kernel files.
No shared Rust changes.
No protocol/state-machine changes.
