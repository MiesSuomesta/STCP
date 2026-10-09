Zephyr MQTT over STCP poll-bypass fix

Problem:
- AF_STCP connection and MQTT gateway work.
- Gateway sees MQTT CONNECT (25 bytes) and MQTT CONNACK (4 bytes).
- Zephyr MQTT loop aborts because zsock_poll() is unsupported/returns EPERM.

Fix:
- remove zsock_poll() dependency from MQTT event loop
- call mqtt_input() directly
- accept -EAGAIN as normal "no data yet"
- call mqtt_live() periodically
- use the same direct service loop after connection for PUBLISH/keepalive

Writes only:
  src/main.c

No module-v2, kernel, shared Rust or gateway changes.
