STCPv2 Zephyr protocol-254 UDP RX fix

Fix:
- RX thread selects datagram handling from carrier->socket_type, not the
  public AF_STCP socket_type.
- Public ABI is SOCK_STREAM/254 while the native/effective carrier is
  SOCK_DGRAM. Using sock->socket_type incorrectly sent protocol-254 RX
  through the stream recv()/stcp_rust_carrier_receive() branch.
- UDP now uses recvfrom() + stcp_rust_carrier_receive_from() and logs peer.

Extract this overlay at zephyr/nordic/ so it replaces:
  module-v2/src/stcp_v2_rx.c

Expected CoAP handshake:
  RXPROBE ... type=dgram public_type=1 carrier_type=2
  RXDIAG UDP CORE ENTER ...
  RXDIAG UDP CORE RETURN ...
and connect should leave handshake wait.
