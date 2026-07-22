# Remote TCP connect fix

## Problem

The platform carrier successfully completed `kernel_connect()` to a remote
STCP listener, but `session::connect()` then searched the process-local
`LISTENERS` registry. That registry contains only listeners created in the
same kernel instance, so cross-host TCP connections incorrectly returned
`-ECONNREFUSED`.

## Fix

For non-UDP sockets with a non-zero platform carrier, `session::connect()` now:

1. records the remote peer;
2. creates the local parser queues;
3. enters `SocketState::Handshake`;
4. resets handshake and reliability state; and
5. returns success so the existing C wrapper can start the STCP handshake.

The old `LISTENERS` rendezvous remains available only when no platform carrier
is attached, preserving the in-kernel loopback backend.

## Rebuild requirement

Rebuild and reload both x86 and Raspberry kernel modules because both link the
same `common-rust` static library.
