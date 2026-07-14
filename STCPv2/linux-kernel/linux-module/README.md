# STCP build files

Copy these files to the module tree:

- `Makefile` -> project root
- `Kbuild` -> project root
- `rust/Cargo.toml` -> Rust crate
- `rust/.cargo/config.toml` -> Rust crate configuration

Expected layout:

```text
linux-module/
├── Makefile
├── Kbuild
├── include/
├── src/
│   ├── stcp_module.c
│   ├── stcp_proto.c
│   ├── stcp_ops.c
│   ├── stcp_rust_ffi.c
│   └── rust_core.o        # generated automatically
└── rust/
    ├── Cargo.toml
    ├── .cargo/config.toml
    └── src/lib.rs
```

Build:

```bash
make LLVM=1 V=1 module
```

Custom kernel tree:

```bash
make \
  KDIR=/path/to/linux \
  LLVM=1 \
  V=1 \
  module
```

Clean kernel objects:

```bash
make LLVM=1 clean
```

Clean everything, including Cargo target:

```bash
make LLVM=1 distclean
```
