# STCPv2 Raspberry Pi Rust core on Zephyr / nRF9151

This package integrates the complete `no_std` Raspberry Pi/Linux Rust STCP
protocol core into the existing Zephyr socket-offload module. C is now the
Zephyr BSD-socket/native-carrier adapter; handshake, framing, encryption,
state, ACK/retransmit and application send/receive are provided by Rust.

## Implemented in this integration

- Raspberry Pi `stcp-kernel-core` copied unchanged except for one required
  external-carrier connect path in `rust/src/session.rs`.
- Cargo `staticlib` build for `thumbv8m.main-none-eabi`.
- Zephyr CMake builds and links `libstcp_kernel_core.a` automatically.
- `CONFIG_STCP_RUST_CORE` build flag.
- C ABI header for all Rust exports.
- Zephyr allocator, panic, wake and debug callbacks.
- PSA X25519 and ChaCha20-Poly1305 callbacks.
- Native TCP/UDP carrier send callback.
- Per-socket carrier RX thread feeding bytes to
  `stcp_rust_carrier_receive()`.
- Client path: socket -> carrier connect -> Rust connect -> Rust handshake ->
  Rust send/recv.
- Wire hexdumps and handshake status logging.

## Current scope

The client TCP path is the first supported milestone and is intended for:

    Zephyr nRF9151 + W5500 -> Linux/Raspberry Pi Rust STCP server

The existing C listener/accept path is retained as a phase-1 fallback. Full
Zephyr Rust-backed server accept and UDP child-carrier creation are marked for
phase 2. Do not treat those server-side paths as complete yet.

## Rust prerequisites

Run once:

```bash
cd stcp-module
bash scripts/setup-rust-toolchain.sh
```

This installs/validates:

- nightly Rust
- `rust-src`
- `thumbv8m.main-none-eabi`

## Build

```bash
cd stcp-mqtt
bash scripts/build-ethernet.sh
bash scripts/flash-ethernet.sh
```

The build script verifies Rust integration before invoking west.

## First test

Start the Linux/Raspberry STCP server on port 19002, then:

```text
stcp config host 192.168.1.20
stcp config port 19002
stcp config transport stcp
stcp config chunk 4096
stcp config total 65536
stcp bench upload
```

Expected log progression:

```text
carrier connect ...
Rust STCP handshake start ...
Rust STCP TX (PublicKey frame)
Rust STCP RX (peer PublicKey / HandshakeDone)
Rust STCP handshake complete ...
```

## Important note

This environment did not contain Rust/Cargo or the complete NCS tree, so the
archive was statically checked but could not be compiled here. The first local
build may expose NCS-version-specific PSA or Zephyr fdtable API differences.
Those should be small adapter-level fixes; the shared Rust protocol core and
external-carrier state path are included.

## NCS 3.3.0 X25519 backend

The nRF9151 TF-M PSA configuration does not expose Montgomery/X25519 key
operations. Enable the bundled RFC 7748 implementation:

```ini
CONFIG_STCP_RUST_X25519_SOFTWARE=y
CONFIG_STCP_RUST_TRACE_CRYPTO=y
```

PSA remains responsible for secure random generation and ChaCha20-Poly1305.
Private keys and shared secrets are never printed.
