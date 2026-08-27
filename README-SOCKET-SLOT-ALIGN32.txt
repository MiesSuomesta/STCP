STCP Zephyr socket-slot / RX-stack 32-byte alignment fix
========================================================

Extract at the STCP repository root:
  cd ~/STCP
  unzip -o stcp-zephyr-socket-slot-align32-fix-20260827.zip

Writes only:
  STCPv2/zephyr/nordic/module-v2/src/stcp_v2_context.c

Why:
- stcp_v2_socket is stored in a static slot array
- rx_stack is embedded in stcp_v2_socket
- observed failing address: rx_stack % 32 == 16
- the rx_stack member offset is itself a multiple of 32
- aligning every socket slot to 32 bytes therefore aligns rx_stack too

Implementation:
- each socket is wrapped in a __aligned(32) slot
- the slot array is also __aligned(32)
- wrapper size is padded to alignment, keeping every array element aligned
- allocation prints sock_mod32 / stack_mod32 once for verification

Expected:
  STCP ALIGN ... sock_mod32=0 stack_mod32=0

No kernel/module changes.
No shared Rust changes.
No protocol changes.
No RX/lifecycle behavior changes.
