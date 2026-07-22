# STCPv2 Raspberry Pi ARM64 port

This source tree targets only 64-bit Raspberry Pi Linux (`aarch64`). The Rust
core uses `aarch64-unknown-none`; x86_64 build support has intentionally been
removed.

## Requirements

- Raspberry Pi OS 64-bit or Debian ARM64
- Kernel headers/build tree matching `uname -r`
- clang, LLVM, LLD, rustup and Rust nightly with `rust-src`
- Kernel crypto modules `libcurve25519` and `libchacha20poly1305`

Run the environment check:

```bash
./raspberry/check-rpi.sh
```

Install the Rust component needed by `-Zbuild-std`:

```bash
rustup toolchain install nightly --component rust-src
```

Build directly on the Raspberry Pi:

```bash
./raspberry/build-rpi.sh
```

Load/unload smoke test:

```bash
./raspberry/smoke-test.sh
```

Full protocol tests can then be run with:

```bash
make test
```

The kernel build tree must exactly match the running kernel. Check with:

```bash
uname -r
make -sC /lib/modules/$(uname -r)/build kernelrelease
```
