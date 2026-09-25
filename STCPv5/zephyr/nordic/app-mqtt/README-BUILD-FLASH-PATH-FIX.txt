STCP Zephyr build/flash path fix
===============================

Use in any duplicated Zephyr application tree, for example:
  application/
  app-mqtt/
  application-mqtt/

Fixes:
- APP_ROOT defaults to the directory containing the scripts.
- STCP root is derived from the Git repository root.
- canonical shared Rust core defaults to:
    <git-top>/STCP/kernel/module/rust
- STCP_SHARED_RUST_CORE_DIR and STCP_CANONICAL_CORE overrides still work.
- resulting sysbuild .config is discovered using the real application name.
- flash uses the same application-local build directory.

No source/protocol/kernel changes.
