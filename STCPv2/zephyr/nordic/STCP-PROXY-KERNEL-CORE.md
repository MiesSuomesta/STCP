# STCPv2 userspace MQTT proxy integration

The proxy now uses the shared current core directly:

    ../../../module/rust/Cargo.toml
    package = stcp-kernel-core

The old `the_stcp_kernel_module` dependency and the v1 `ProtoSession` /
`rust_session_handshake_lte` API are no longer used by the proxy.

To make the no_std kernel core usable from a normal Linux Rust binary, the
core gains a `userspace` feature which:
- builds an `rlib` in addition to the kernel/Zephyr `staticlib`;
- disables the kernel-owned global allocator/panic handler;
- exports the small FFI surface required by the proxy;
- adds `stcp_rust_create_accepted_stream()` for an already accepted external
  TCP carrier.

The proxy supplies Linux userspace implementations of the carrier and crypto
callbacks expected by the shared core. The wire protocol therefore comes from
the exact same `stcp-kernel-core` sources used by the kernel/Zephyr builds.

Expected source layout for the relative Cargo path:

    nordic/
      application/
        testing/
          rust/       <- this proxy
      module/
        rust/         <- stcp-kernel-core

Build:

    cd application/testing/rust
    cargo build --release --bin stcp-proxy

Run Mosquitto locally on TCP/1883, then:

    ./target/release/stcp-proxy

The proxy listens for the STCP carrier on TCP/7777 and forwards decrypted MQTT
bytes to 127.0.0.1:1883.
