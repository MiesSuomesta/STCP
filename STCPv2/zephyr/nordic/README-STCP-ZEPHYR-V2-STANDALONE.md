# STCPv2 Zephyr clean/standalone application

This overlay is meant to be extracted into the STCP root, e.g. `~/zephyr-stcp/stcp`.
It provides a real buildable `application/`, a parallel `module-v2/`, Robot tests, and
only a Cargo target config under `kernel/module/rust/.cargo/`. It does **not** replace
the canonical Rust core source in `kernel/module/rust`.

## Design

- Canonical protocol/session/crypto/reliability logic: `kernel/module/rust`
- Zephyr adapter only: `module-v2`
- AF_STCP is registered with `NET_SOCKET_REGISTER`, **not** generic socket offload.
- Native AF_INET TCP/UDP remains on Zephyr's native stack.
- No legacy `CONFIG_STCP`, `CONFIG_STCP_RUST_*`, or global `NET_SOCKETS_OFFLOAD` dependency.
- Useful old implementation pieces retained: socket vtable/provider shape, native TCP/UDP
  carrier, RX pump, PSA crypto glue, software X25519, shell/benchmark application.
- Stream server-side `accept()` is intentionally not re-created with the old pseudo-child
  logic; it waits for a proper shared-core accepted-stream API. Client matrix is supported.

## W5500 release workaround

The NCS 3.3.0 W5500 driver used in this setup loses the INTn IRQ path. The build script
applies the proven polling fallback from `application/scripts/patch-w5500-driver.py`.
It saves the pristine Zephyr driver as `eth_w5500.c.stcp-original` and can be disabled:

```bash
STCP_W5500_POLLING=0 bash application/scripts/build-v2-clean.sh
```

## Build

From an activated NCS/west environment:

```bash
cd ~/zephyr-stcp/stcp/application
bash scripts/build-v2-clean.sh
```

The build goes to `application/build-v2-clean` and checks that:

```text
CONFIG_STCP_V2=y
CONFIG_NET_NATIVE=y
CONFIG_NET_SOCKETS_OFFLOAD is not set
```

Flash:

```bash
bash scripts/flash-v2-clean.sh
```

## Robot

The Linux side must have the STCP kernel module loaded and AF_STCP available.

```bash
cd ~/zephyr-stcp/stcp/application/testing/robot-v2
bash setup-venv.sh
source .venv/bin/activate

export STCP_ZEPHYR_SERIAL=/dev/ttyACM0
export STCP_ZEPHYR_SERVER_HOST=192.168.1.20
bash ../../scripts/run-zephyr-v2-robot.sh
```

Initial matrix:

1. native Ethernet ping to the Linux host
2. STCPv2 upload to Linux AF_STCP BEN2 server
3. STCPv2 download
4. STCPv2 full duplex

Bring-up defaults are 256 KiB / 4 KiB. After a green run:

```bash
export STCP_ZEPHYR_TOTAL=1048576
export STCP_ZEPHYR_CHUNK=16384
bash ../../scripts/run-zephyr-v2-robot.sh
```
