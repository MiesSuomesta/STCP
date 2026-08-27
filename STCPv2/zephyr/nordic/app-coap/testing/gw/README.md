STCPv2 CoAP Gateway
===================

Ported directly from the known-working MQTT gateway structure.

Linux STCP side:
  AF_STCP
  SOCK_STREAM
  protocol 254
  bind/listen/accept

CoAP backend:
  UDP 127.0.0.1:5683

Flow:
  Zephyr CoAP
    -> STCP-UDP
    -> Linux AF_STCP proto 254 listener
    -> UDP libcoap server
    -> response back over the accepted STCP session

Defaults:
  STCP listen : 0.0.0.0:56830
  CoAP target : 127.0.0.1:5683

Build:
  cargo build --release

Run:
  ./target/release/stcp-v2-coap-gateway

Optional:
  ./target/release/stcp-v2-coap-gateway 0.0.0.0 56830 127.0.0.1:5683

Important:
Linux STCP-UDP currently uses SOCK_STREAM + protocol 254.  Do not change the
Linux gateway socket to SOCK_DGRAM; the current kernel provider rejects that
with EPROTONOSUPPORT / socket type not supported.
