STCPv2 userspace test-server — current canonical core
===================================================

Fixes the test server for the current STCPv2 canonical Rust core.

Problem fixed
-------------
The canonical core has:

    mod ffi;

and ffi.rs exports the stcp_rust_* functions as `pub extern "C"`, but the
private `ffi` module means those functions are not visible from crate root.

This overlay adds userspace-only re-exports in:

    kernel/module/rust/src/lib.rs

Only the symbols required by the test server are re-exported, and only when
the `userspace` Cargo feature is enabled. Kernel/no_std builds are unchanged.

The current core also delegates session-key derivation through:

    stcp_kernel_derive_session_keys(...)

The userspace test server now implements that hook with SHA256 + HMAC-SHA256,
using exactly the same KDF byte layout as the Linux/Zephyr platform crypto
implementation.

Files
-----
kernel/module/rust/src/lib.rs
zephyr/nordic/application/testing/test-server/Cargo.toml
zephyr/nordic/application/testing/test-server/bin/stcp_server.rs

Build
-----
cd /home/pomo/git/github/STCP/STCPv2/zephyr/nordic/application/testing/test-server
cargo clean
cargo build --release

The server still uses the real canonical v2 FFI API; no parallel protocol
implementation is introduced.
