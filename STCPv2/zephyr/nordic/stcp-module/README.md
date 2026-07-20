# STCP Zephyr socket-offload stub

Out-of-tree Zephyr module implementing a stub BSD socket provider for:

- address family `AF_STCP` = 45
- socket type `SOCK_STREAM`
- protocol `STCP_PROTO` = 253

The package contains no Rust and no real STCP carrier. It provides the integration skeleton for Zephyr's BSD socket dispatcher and file-descriptor table.

## Implemented stub operations

- `socket()` creates a real Zephyr socket file descriptor
- `bind()` stores the local STCP address
- `listen()` moves the context to listener state
- `connect()` succeeds immediately and stores the peer address
- `send()` reports all bytes as accepted but does not transport them
- `recv()` returns `EAGAIN` because there is no carrier
- `accept()` returns `EAGAIN` because there are no incoming connections
- `getsockname()`, `getpeername()`, `SO_ERROR`, `shutdown()`, `close()`
- basic `fcntl(F_GETFL/F_SETFL)` nonblocking state

`poll()` integration and real blocking waits are intentionally deferred until the carrier/core exists.

## Add to an existing build

```sh
west build -p always -b native_sim/native path/to/app -- \
  -DZEPHYR_EXTRA_MODULES=/absolute/path/to/stcp-zephyr-offload-stub
```

Or run:

```sh
./scripts/build-native.sh
./build/zephyr/zephyr.exe
```

## Nordic board example

```sh
west build -p always -b nrf52840dk/nrf52840 samples/stcp_offload_stub -- \
  -DZEPHYR_EXTRA_MODULES="$PWD"
```

## Compatibility note

Zephyr's internal FD/vtable API can change between NCS releases. This skeleton targets the current `socket_op_vtable`, `zvfs_reserve_fd()` and `zvfs_finalize_typed_fd()` model. If your checkout reports a signature or vtable member mismatch, compare against:

```sh
grep -R "struct socket_op_vtable" "$ZEPHYR_BASE/include" -n
grep -R "NET_SOCKET_OFFLOAD_REGISTER" "$ZEPHYR_BASE/drivers" "$ZEPHYR_BASE/subsys" -n
grep -R "zvfs_finalize_typed_fd" "$ZEPHYR_BASE/include" -n
```

The next implementation step is to replace `src/stcp_core_stub.c` with the portable C protocol core and add a carrier RX/TX adapter.
