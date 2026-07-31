# Release-mode changes

- `STCP_RELEASE=y` is now the default for Raspberry Pi and x86 module builds.
- Rust core still uses Cargo `--release`; release builds now disable forced frame pointers.
- Verbose STCP trace sites are compiled out through `STCP_TRACE*` macros.
- Hot-path logging removed from release code includes TCP TX begin/result, RX byte tracing,
  carrier send tracing, connect/accept tracing and `stcp_kernel_debug_event()` output.
- Kernel errors (`pr_err*`) and module load/unload messages remain enabled.

Release build:

```bash
make clean
make -j"$(nproc)"
```

Diagnostic build with trace sites enabled:

```bash
make clean
make STCP_RELEASE=n -j"$(nproc)"
```
