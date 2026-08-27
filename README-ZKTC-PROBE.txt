STCP Zephyr k_thread_create probe

Extract from repository root:
  cd ~/STCP
  unzip -o stcp-zephyr-zktc-thread-create-probe-20260827.zip

Writes only:
  STCPv2/zephyr/nordic/module-v2/src/stcp_v2_rx.c

Markers:
  ZKTC BEFORE ...
  ZKTC AFTER ...
  ZKTC THREAD_ENTER ...
  ZKTC THREAD_READY ...

The BEFORE marker includes:
- socket/thread/stack addresses
- K_KERNEL_STACK_SIZEOF(rx_stack)
- stack address modulo 32 (align32)
- rx_running/rx_stop

Diagnostic only. No kernel/shared Rust/struct/protocol changes.
