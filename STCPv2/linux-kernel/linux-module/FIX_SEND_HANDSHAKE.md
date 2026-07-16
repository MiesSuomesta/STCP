# Blocking send handshake fix

The carrier handshake is asynchronous. Previously `stcp_sendmsg()` returned
Rust's internal `-EAGAIN` immediately when userspace called `send()` directly
after `connect()` or `accept()`.

The fixed wrapper now:

- preserves `MSG_DONTWAIT` behavior,
- waits interruptibly on `recv_wq` for `stcp_rust_is_connected()`,
- times out after 10 seconds instead of hanging forever,
- retries the same send after the session reaches `Ready`.
