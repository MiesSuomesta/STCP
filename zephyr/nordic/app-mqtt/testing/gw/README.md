# STCP MQTT Gateway

This is the v2 replacement for the old v1 gateway/proxy code.

The v1 server manually instantiated the old Rust `ProtoSession`, performed the
STCP handshake in userspace, decrypted frames and forwarded them to TCP.

STCP already provides that transport through the Linux kernel AF_STCP socket
API, so the gateway is intentionally much smaller:

```text
Zephyr MQTT library
    |
    | AF_STCP / SOCK_STREAM / proto 253
    v
Linux STCP kernel module
    |
    | plaintext byte stream
    v
stcp-v2-mqtt-gateway
    |
    | ordinary TCP
    v
Mosquitto :1883
```

## Requirements

The tested STCP Linux kernel module must already be loaded.

Verify:

```sh
lsmod | grep '^stcp '
modinfo stcp
```

Mosquitto should be listening locally:

```sh
ss -ltnp | grep ':1883'
```

## Build

```sh
cargo build --release
```

## Run

Defaults:

- STCP listen: `0.0.0.0:18830`
- backend: `127.0.0.1:1883`

```sh
./target/release/stcp-v2-mqtt-gateway
```

Or explicitly:

```sh
./target/release/stcp-v2-mqtt-gateway \
    0.0.0.0 18830 127.0.0.1 1883
```

Configure Zephyr MQTT app to use the Linux host and STCP gateway port:

```text
CONFIG_STCP_MQTT_BROKER_IPV4="192.168.1.20"
CONFIG_STCP_MQTT_BROKER_PORT=18830
```

Observe publishes:

```sh
mosquitto_sub -h 127.0.0.1 -p 1883 -t 'stcp/demo' -v
```

## Design

No v1 STCP Rust core dependency remains.

No crypto or handshake is implemented in this process. The kernel's known-good
STCP implementation owns handshake, encryption, retransmission and stream
semantics.

The gateway only performs:

1. `socket(AF_STCP, SOCK_STREAM, 253)`
2. `bind/listen/accept`
3. TCP connect to Mosquitto
4. bidirectional byte copy
