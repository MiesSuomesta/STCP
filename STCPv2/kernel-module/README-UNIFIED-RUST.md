# STCPv2 Linux kernel ports with one shared Rust core

This package contains two kernel-module projects and exactly one Rust source tree:

```text
STCPv2-linux-kernel-unified-rust/
├── common-rust/                 # the only Rust source tree
├── x86-kernel-module/           # x86_64 Linux kernel adapter and build
└── raspberry-kernel-module/     # Raspberry Pi ARM64 adapter and build
```

Both modules compile `common-rust` as a `no_std` static library with `core` and
`alloc`. Each Makefile selects its own target and generates a port-local
`src/rust_core.o` before Kbuild links `stcp.ko`.

## x86_64 build

```bash
cd x86-kernel-module
make KDIR=/lib/modules/$(uname -r)/build -j$(nproc)
```

The x86 build uses:

```text
RUST_TARGET=x86_64-unknown-none
RUST_DIR=../common-rust
```

## Raspberry Pi ARM64 cross-build

```bash
cd raspberry-kernel-module
make \
  KDIR=/path/to/raspberry-kernel-sources \
  CROSS_COMPILE=aarch64-linux-gnu- \
  -j$(nproc)
```

The Raspberry build uses:

```text
RUST_TARGET=aarch64-unknown-none
RUST_DIR=../common-rust
```

## Override shared Rust location

Both Makefiles support an explicit override:

```bash
make RUST_DIR=/absolute/path/to/common-rust ...
```

## Important

Do not create port-specific copies of `common-rust`. Platform-specific Linux
kernel operations remain in each module project's C adapter (`src/`), while
protocol, state, framing, reliability and crypto logic remain shared.
