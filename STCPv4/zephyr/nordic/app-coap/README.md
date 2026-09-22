# STCPv2 CoAP UDP Gateway

Datagram-model gateway for CoAP over STCPv2.

Linux STCP socket:
  AF_STCP
  SOCK_DGRAM
  IPPROTO_STCP (253)

The STCP implementation maps the datagram socket type to the STCP-UDP protocol.

Flow:

Zephyr CoAP
  -> AF_STCP / SOCK_DGRAM / IPPROTO_STCP
  -> Linux STCPv2 gateway :56830
  -> UDP 127.0.0.1:5683
  -> libcoap server

No listen()/accept() is used.

Build:

  cargo build --release

Run defaults:

  ./target/release/stcp-v2-coap-gateway

Defaults:
  STCP listen: 0.0.0.0:56830
  backend:     127.0.0.1:5683

Optional arguments:

  ./target/release/stcp-v2-coap-gateway 0.0.0.0 56830 127.0.0.1:5683
