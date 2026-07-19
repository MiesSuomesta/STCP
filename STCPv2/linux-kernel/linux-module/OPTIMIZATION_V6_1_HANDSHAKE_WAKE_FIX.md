# STCPv2 V6.1 handshake wake fix

## Regression fixed

V6 removed the carrier receive thread's C-side `stcp_kernel_wake_recv()` call.
That wake is required as a fallback while a newly connected/accepted Rust
context does not yet have a valid socket owner. Without it, the first
handshake/data receive can remain queued while `sendmsg()` sleeps waiting for
the socket to become writable. The observed result is connections succeeding
but all first sends and server sessions failing.

The C-side wake has been restored. It is guarded by `waitqueue_active()` in
`stcp_kernel_wake_recv()`, so the idle fast path remains inexpensive.

## Conservative TX correction

The DATA path again calls `send_frame()` instead of bypassing it with a direct
`carrier::transmit()` invocation. This keeps carrier lookup and frame validation
in one established path and avoids stale carrier-pointer assumptions.

## Retained optimizations

- chunked `ByteQueue` reads/discards
- owned plaintext `Vec` insertion without payload copy
- block handshake key read
- partial-tail chunk reuse
- kernel 7.2 `kvfree_sensitive(buffer, buffer_size)` API fix
