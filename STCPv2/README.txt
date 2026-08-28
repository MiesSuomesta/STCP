STCPv2 X25519 silent shared-secret overlay

Apply on top of the current quiet-stackdiag source.

Changed:
  zephyr/nordic/module-v2/src/stcp_v2_platform.c

Only diagnostic I/O is removed from stcp_kernel_x25519_shared():
  - start LOG_INF
  - X25519STACK measurement/printk
  - shared-secret error LOG_ERR lines
  - optional shared-secret hexdump
  - completion LOG_INF

Preserved:
  - software X25519 call
  - reset-stage writes 1..4
  - error returns and shared buffer zeroization
  - all-zero result rejection
  - all other platform/socket/RX/carrier behavior

The current stcp_x25519_soft.c is not included, so the existing quiet
software-X25519 implementation remains untouched.
