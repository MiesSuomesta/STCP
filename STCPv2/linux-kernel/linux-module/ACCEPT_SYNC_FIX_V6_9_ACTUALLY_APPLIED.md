# V6.9 TCP accept queue synchronisation fix

The previous package documentation claimed that the inner carrier accept was
blocking, but `src/stcp_ops.c` still passed the public listener's `flags` to
`stcp_carrier_accept()`.

With a nonblocking STCP listener this allowed the following race:

1. `stcp_rust_accept()` pops the logical Rust child.
2. `kernel_accept(..., O_NONBLOCK)` transiently returns `-EAGAIN`.
3. The popped Rust child is released.
4. The matching TCP child remains queued.
5. Rust and TCP accept queues are permanently out of sync; clients never reach
   handshake/Ready and the stress test reports zeros.

V6.9 passes `0` to the inner carrier accept after the Rust child is secured.
The public STCP listener remains poll-driven and nonblocking; only the private
matching TCP accept is blocking and atomic with respect to the popped Rust child.
