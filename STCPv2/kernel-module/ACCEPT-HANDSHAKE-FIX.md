# Remote TCP accept / handshake fix

## Problem

The client TCP carrier connected successfully, but the Raspberry listener never
called `kernel_accept()`. The old flow asked Rust for a queued accepted child
before accepting the underlying TCP socket, creating a circular dependency.

## New TCP flow

1. C requests a provisional server-side Rust context through
   `stcp_rust_create_stream_accepted_child()`.
2. C calls `kernel_accept()` through `stcp_carrier_accept()`.
3. The accepted carrier is attached to the Rust child.
4. The carrier receive thread is started.
5. The server handshake is started.
6. C waits for the child to reach the connected/ready state before returning
   the userspace `accept()` call.

UDP keeps using the Rust logical accept queue because UDP children are created
from the first received datagram.

## Added interfaces

- `stcp_rust_create_stream_accepted_child()`
- `stcp_carrier_is_udp()`

Both x86 and Raspberry kernel-module adapters contain the same flow and use the
single `common-rust/` source tree.
