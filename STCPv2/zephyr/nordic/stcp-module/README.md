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
