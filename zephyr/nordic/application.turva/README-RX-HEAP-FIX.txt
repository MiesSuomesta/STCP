STCP Zephyr RX stack fix

Changes:
- CONFIG_STCP_V2_RX_BUFFER_SIZE is allocated with k_malloc() per socket.
- stcp-v2-rx thread keeps only a pointer/size on its stack.
- buffer is released in stcp_v2_socket_free().
- TX path was audited: module-v2 has no equivalent CONFIG-sized stack buffer.
  stcp_v2_carrier_send_wire() remains direct/zero-copy into zsock_send/sendto.

This intentionally does NOT change canonical Rust core source.
