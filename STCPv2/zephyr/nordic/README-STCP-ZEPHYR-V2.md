# STCPv2 Zephyr clean adapter

This is a parallel, clean Zephyr implementation. It does not replace the old
`module/` until the Robot matrix is green.

## Design

The canonical protocol implementation is `kernel/module/rust` — the same Rust
core used by Linux/Raspberry Pi. Zephyr contains only platform glue:

* AF_STCP socket-family provider and ZVFS vtable
* native TCP/UDP carrier adapter
* RX pump feeding wire bytes into the Rust core
* PSA/Zephyr crypto and allocator hooks
* optional UDP accepted-child carrier glue

There is no duplicate Zephyr protocol/session implementation and no v1
connection manager.

## Important socket-provider change

The old module used `NET_SOCKET_OFFLOAD_REGISTER()` and selected
`CONFIG_NET_SOCKETS_OFFLOAD`. That globally changed normal AF_INET socket
selection and interfered with the W5500/native stack.

The clean implementation keeps the useful custom socket-provider/vtable model,
but registers AF_STCP with `NET_SOCKET_REGISTER()`. AF_STCP is therefore a
custom family, not a generic network-device socket offload.

## Current scope

Client TCP and UDP are implemented. UDP listen/accept is wired through the
shared core. Stream `accept()` intentionally returns `EOPNOTSUPP` until the
canonical shared core exposes an accepted-stream constructor; the old fake
C-side connected child is deliberately not copied.

This is intentional: no v1 behavior is reintroduced merely to make an API look
complete.

## Build without replacing the old module

From `zephyr/nordic/application`:

```bash
bash scripts/build-v2-clean.sh
```

It uses `module-v2` through `ZEPHYR_EXTRA_MODULES`, sets `CONFIG_STCP=n`, and
sets `CONFIG_STCP_V2=y`.

The default Rust path is the repository's `kernel/module/rust`. Override only
when necessary:

```bash
export STCP_SHARED_RUST_CORE_DIR=/path/to/STCPv2/kernel/module/rust
```

## Robot

The Robot test server uses the Linux AF_STCP kernel socket directly and speaks
the BEN2 protocol expected by the Zephyr benchmark application. Thus the test
is genuinely Zephyr STCPv2 -> Linux golden kernel core.

```bash
cd application/testing/robot-v2
python3 -m venv .venv
. .venv/bin/activate
pip install -r requirements.txt

export STCP_ZEPHYR_SERIAL=/dev/ttyACM0
export STCP_ZEPHYR_SERVER_HOST=192.168.1.20
bash ../../scripts/run-zephyr-v2-robot.sh
```

Default Robot payload is deliberately 256 KiB / 4 KiB chunks for bring-up.
Raise it to the normal 1 MiB / 16 KiB after the smoke matrix is green:

```bash
export STCP_ZEPHYR_TOTAL=1048576
export STCP_ZEPHYR_CHUNK=16384
```

Do not delete the old module until this matrix and the existing Linux/RPi
16/16 regression matrix are both green.
