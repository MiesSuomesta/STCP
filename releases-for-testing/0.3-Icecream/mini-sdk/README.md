# STCP mini SDK

A deliberately small public Rust example for the Linux STCP socket ABI.
It does **not** depend on the full/private STCP SDK.

Requirements:
- Linux with a compatible STCP kernel module loaded
- Rust 1.85+ / Cargo

Build:

```sh
cargo build --release
```

Run server:

```sh
./target/release/stcp-echo-server 0.0.0.0:19002
```

Run client:

```sh
./target/release/stcp-echo-client 127.0.0.1:19002 "Hello STCP"
```

The mini SDK uses the public socket ABI directly:
- `AF_STCP = 45`
- `STCP_TCP = 253`
- `STCP_UDP = 254` (constant exposed; this first minimal example uses STCP-TCP)
