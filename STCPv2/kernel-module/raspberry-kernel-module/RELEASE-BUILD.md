# STCP release build

Release mode is the default:

```bash
make clean
make -j$(nproc)
```

It sets `STCP_RELEASE=1`, builds Rust with Cargo `--release`, disables Rust
frame pointers, and compiles verbose STCP trace/printk call sites out. Kernel
errors (`pr_err*`) and module load/unload status messages remain enabled.

For an explicit diagnostic build:

```bash
make clean
make STCP_RELEASE=n -j$(nproc)
```

In diagnostic builds, the carrier-specific runtime parameter can be enabled:

```bash
sudo modprobe stcp carrier_debug=1
```
