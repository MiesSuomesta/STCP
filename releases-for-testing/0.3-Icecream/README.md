# STCP public release bundle

This directory is intended to be publishable without the private/full SDK repository.

Contents:
- `STCP/version-to-use/` — snapshot of the selected public STCP source tree.
- `mini-sdk/` — standalone Rust mini SDK and simple STCP echo server/client.

Quick build:

```sh
cd mini-sdk
cargo build --release
```

The Rust examples compile independently of the STCP kernel source. At runtime they require a compatible STCP kernel module to be installed and loaded.
