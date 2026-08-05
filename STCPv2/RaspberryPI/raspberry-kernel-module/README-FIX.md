# Raspberry STCP-TCP listener matching fix

This source bundle fixes local listener lookup for wildcard binds.

A server bound to `0.0.0.0:PORT` must match a client connecting to the
machine's concrete IPv4 address on the same port. The previous exact
`Address` comparison incorrectly sent Raspberry-to-Raspberry STCP-TCP
connections into the external-peer path, where the handshake timed out.

Changed source files:

- `rust/src/session.rs` (active FFI connect path)
- `rust/src/transport.rs` (kept consistent for alternate/internal users)

The match now requires equal ports and accepts either an exact IPv4 match or
a wildcard listener address (`addr == 0`).
