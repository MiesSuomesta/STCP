# STCP RX context race hardening (debug v7)

The observed `rust_ctx=0` log was produced by an older module build: the current
source already drops NULL-context RX before Rust dispatch. This revision makes
the invariant impossible to miss:

- module diagnostics are tagged `stcp-debug-v7`
- accepted carrier owner is attached before `rust_ctx` publication
- `stcp_carrier_set_rust_ctx()` logs the exact carrier/context pair
- the receiver thread refuses to start if `rust_ctx == NULL`
- NULL context is never passed to `stcp_rust_carrier_receive_from()`

Use a clean rebuild and verify the loaded module:

```bash
make clean
make -j$(nproc)
sudo modprobe -r stcp
sudo insmod ./stcp.ko
dmesg | tail -100 | grep stcp-debug-v7
```

Expected ordering:

1. `carrier rust_ctx wired`
2. `RX start validated`
3. `RX thread enter`
4. `RX bytes ... rust_ctx=<non-zero>`
