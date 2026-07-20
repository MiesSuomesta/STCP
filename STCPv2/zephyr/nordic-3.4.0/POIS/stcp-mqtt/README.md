# STCP Zephyr C port

Zephyr 4.4 / nRF Connect SDK OOT socket-offload module derived from the Linux
module's C architecture. Rust is neither included nor modified.

## Build nRF9151 DK

```bash
source ../.venv/bin/activate
./scripts/build-nrf9151.sh
./scripts/flash-nrf9151.sh
```

Recover a protected target once if required:

```bash
./scripts/flash-nrf9151.sh --recover
```

Serial console: 115200 8N1 on one of the J-Link `/dev/ttyACM*` ports.

## API

```c
int fd = zsock_socket(AF_STCP, SOCK_STREAM, STCP_PROTO_TCP);
struct sockaddr_in addr = { .sin_family = AF_INET, ... };
zsock_connect(fd, (struct sockaddr *)&addr, sizeof(addr));
zsock_send(fd, data, len, 0);
zsock_recv(fd, buf, sizeof(buf), 0);
zsock_close(fd);
```

See `docs/PORTING.md` for limitations and Linux-to-Zephyr mapping.

## nRF9151 build and flash scripts

Build the application-core firmware for the nRF9151 DK:

```bash
./scripts/build-modem.sh
```

Flash it with the default `nrfutil` runner:

```bash
./scripts/flash-modem.sh
```

If AP-Protect is enabled, recover and flash:

```bash
./scripts/flash-modem.sh --recover
```

Select a specific probe or J-Link runner:

```bash
./scripts/flash-modem.sh --serial 1052094012
./scripts/flash-modem.sh --runner jlink
```

The build script compiles the Zephyr application for the nRF9151 application
core. Nordic's modem firmware is a separately supplied binary and is not built
from this project.

## Connect/send/recv test

The sample now performs a real blocking TCP-carrier round trip through the
`AF_STCP` offload provider:

1. `zsock_socket(AF_STCP, SOCK_STREAM, 253)`
2. `zsock_connect()`
3. `zsock_send()` until all bytes are accepted
4. `zsock_recv()` with a receive timeout
5. `zsock_shutdown()` and `zsock_close()`

Defaults are in `samples/stcp_c_port/prj.conf`. The remote endpoint must be a
TCP echo server reachable through an initialized Zephyr network interface.
A simple Linux echo server can be started with:

```bash
socat -v TCP-LISTEN:6677,reuseaddr,fork EXEC:/bin/cat
```

For nRF9151, cellular network attachment must be initialized by the application
before `connect()` can succeed. This package keeps that board/modem setup
separate from the STCP socket offload layer.


## LTE-enabled nRF9151 test

`scripts/build-modem.sh` now adds `samples/stcp_c_port/nrf9151.conf`, initializes the nRF modem library, waits for LTE registration, and only then opens the STCP socket. The hidden TCP carrier handles asynchronous `connect()` completion with `zsock_poll()` and `SO_ERROR`. The sample no longer depends on `SO_RCVTIMEO`.

## Reused STCPv1 LTE initialization

The nRF9151 build now uses `src/stcp_lte.c` instead of directly calling
`nrf_modem_lib_init()` and `lte_lc_connect()` from the sample. The implementation
reuses the useful C-only behaviour from the supplied STCPv1 transport:

- LTE registration event tracking
- default PDN activation tracking
- IPv4 readiness verification with `AT+CGPADDR`
- RRC/radio state tracking
- semaphores for LTE, PDN and IP readiness
- modem status dump commands on startup failure

STCPv1 FSM, refcount, workers, DNS subsystem and all Rust calls are intentionally
left out of this port.

## STCP echo server

Build and flash the nRF9151 echo server:

```bash
./scripts/build-echo-server.sh
./scripts/flash-echo-server.sh
```

The server listens on carrier TCP port 6677 through the AF_STCP socket provider.
It accepts one client at a time and echoes every received byte unchanged.

The application loop is intentionally separated into `serve_client()` so that
future protocol dispatch can route traffic to CoAP, MQTT, or raw echo handlers.

## STCP2 internet proxy client

The nRF9151 acts as an outbound proxy client and maintains a connection to
`lja.fi:7777`. This avoids inbound LTE/CGNAT limitations. The current test
mode sends an online banner and echoes received bytes. Replace the raw echo
section later with CoAP and MQTT channel dispatch.

```bash
./scripts/build-proxy-client.sh
./scripts/flash-proxy-client.sh
```

## NCS 3.4 nRF Modem include fix

The module CMake explicitly exports the sibling workspace path
`nrfxlib/nrf_modem/include` when `CONFIG_NRF_MODEM_LIB=y`. This is required by
this OOT build so `nrf/lib/at_monitor` can include `nrf_modem_at.h`.

## STCP2 MQTT proxy

The `samples/stcp_mqtt_proxy` sample implements MQTT 3.1.1 directly over an
`AF_STCP` stream. It connects to `lja.fi:7777`, subscribes to
`stcp2/nrf9151/down`, publishes status/data to `stcp2/nrf9151/up`, sends
PINGREQ keepalives and reconnects with exponential backoff and jitter.

Build and flash:

```sh
./scripts/build-mqtt-proxy.sh
./scripts/flash-mqtt-proxy.sh
```
