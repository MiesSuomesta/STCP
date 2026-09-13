# Zephyr CoAP over STCPv2

This project keeps the working MQTT/STCP baseline infrastructure and replaces
only the application protocol with Zephyr's CoAP library.

Transport:
- AF_STCP=45
- SOCK_STREAM
- proto 254 (STCP-UDP variant)
- gateway listen 56830
- UDP backend 127.0.0.1:5683

Zephyr sends a confirmable GET to:
  /hello

Gateway:
  testing/gw

Build gateway:
  cd testing/gw
  cargo build

Run gateway:
  ./target/debug/stcp-v2-coap-gateway

Linux needs a CoAP UDP server on 127.0.0.1:5683.

The Zephyr application logs response code and payload.
