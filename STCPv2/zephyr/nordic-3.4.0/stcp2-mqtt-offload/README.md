# STCP2 MQTT offload package

- `stcp-module/`: Zephyr OOT socket-offload provider for `AF_STCP=45`, protocol 253/254.
- `stcp-mqtt/`: application depending on `../stcp-module` via `ZEPHYR_EXTRA_MODULES`.

The MQTT application reuses the STCPv1 design ideas (reconnect/backoff, subscribe/publish, QoS1 ACK and keepalive) but removes the old refcount/FSM/patched Zephyr MQTT transport dependencies. MQTT 3.1.1 is encoded directly over the normal BSD `AF_STCP` socket.

Build:
```bash
cd stcp-mqtt
./scripts/build.sh
```
Flash:
```bash
./scripts/flash.sh
```
