STCPv2 Zephyr protocol-254 UDP ABI fix
======================================

Goal
----
Make Zephyr expose the same public STCPv2 socket ABI as Linux/SDK without
changing the known-good TCP/253 path.

Canonical public ABI after this overlay:

  AF_STCP + SOCK_STREAM + 253 -> STCP-TCP
  AF_STCP + SOCK_STREAM + 254 -> STCP-UDP

Backward compatibility retained:

  AF_STCP + SOCK_DGRAM + 253 -> STCP-UDP
  AF_STCP + SOCK_DGRAM + 254 -> STCP-UDP

Internal Zephyr behavior for protocol 254:
- effective socket type: SOCK_DGRAM
- native carrier: AF_INET / SOCK_DGRAM / UDP
- Rust protocol: 254
- RX uses recvfrom + stcp_rust_carrier_receive_from
- send-wire uses sendto when peer is known

Files:
  module-v2/include/stcp/stcp.h
  module-v2/src/stcp_v2_socket.c
  app-coap/src/coap_stcp_transport.c

No Linux kernel changes.

Expected CoAP create log:
  public_type=1 protocol=254 effective_type=2 rust_protocol=254

This deliberately leaves MQTT/STCP-TCP:
  SOCK_STREAM + 253 -> effective_type=SOCK_STREAM / Rust 253
unchanged.
