Zephyr CoAP STCP-UDP socket contract fix

Changes only:
  src/coap_stcp_transport.c

Correct Zephyr STCP socket signature:
  socket(AF_STCP, SOCK_DGRAM, IPPROTO_STCP)

The Zephyr adapter itself maps:
  SOCK_STREAM -> Rust protocol 253
  SOCK_DGRAM  -> Rust protocol 254

Therefore passing protocol 254 directly was incorrect on Zephyr.

Linux gateway remains AF_STCP / SOCK_STREAM / protocol 254 according to the
current Linux/SDK ABI. No gateway or shared-core change in this overlay.
