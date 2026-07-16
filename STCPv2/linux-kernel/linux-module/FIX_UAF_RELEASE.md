# Carrier release UAF fix

Fixed the KASAN-reported `stcp_carrier_send()` use-after-free during close:

- Rust `session::release()` is now local teardown only and never sends a Close frame.
- C detaches Rust owner/carrier pointers before stopping and freeing the carrier.
- Carrier receiver thread is stopped before the Rust context is freed.
- Explicit `shutdown()` sends the protocol Close frame before shutting down the carrier socket.
- Initial sockets now initialize retransmit delayed work just like accepted children.
- Accept error paths detach and free child resources in a safe order.
- Corrected the C declaration of `stcp_rust_shutdown()` to `void`.
