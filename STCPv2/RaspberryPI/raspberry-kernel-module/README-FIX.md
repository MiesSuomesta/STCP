# Raspberry STCP external TCP handshake fix

This archive contains replacement source files only. It does not contain patch files or build artifacts.

Changes:

- `rust/src/carrier.rs`
  - imports `Role`
  - external STCP-TCP server children adopt the non-zero connection ID from the first complete `PublicKey` frame
  - wakes the child wait queue after adoption
- `rust/src/ffi.rs`
  - exports `stcp_rust_connection_id()`
- `include/stcp_rust_ffi.h`
  - declares `stcp_rust_connection_id()`
- `src/stcp_ops.c`
  - waits for the external child's connection ID before starting the server handshake
  - logs and returns `ETIMEDOUT` if the first PublicKey frame never arrives

Extract this zip directly over the root of `raspberry-kernel-module/`, then clean-build the module/kernel.
