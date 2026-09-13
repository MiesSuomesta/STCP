STCPv2 userspace test-server overlay
====================================

This overlay changes only:
  kernel/module/rust/Cargo.toml
  kernel/module/rust/src/lib.rs
  zephyr/nordic/application/testing/test-server/Cargo.toml
  zephyr/nordic/application/testing/test-server/bin/stcp_server.rs

Important:
- Default kernel build remains no_std + staticlib.
- userspace is opt-in via feature "userspace".
- The server uses the actual v2 API:
    stcp_rust_create()
    stcp_rust_bind()/listen()
    stcp_rust_create_external_tcp_child()
    stcp_rust_set_carrier()
    stcp_rust_carrier_receive()
    stcp_rust_recv()/send()
- There is no create_accepted_stream helper.
- One OS TCP connection is handled at a time.

Build:
  cd zephyr/nordic/application/testing/test-server
  cargo clean
  cargo build --release

Run:
  ./target/release/stcp-server
