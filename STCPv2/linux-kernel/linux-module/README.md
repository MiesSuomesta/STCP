# STCP carrier selection by socket protocol number

Carrier selection is now per BSD socket, using the third argument of
`socket()`.

```c
socket(AF_STCP, SOCK_STREAM, STCP_PROTO_TCP);
socket(AF_STCP, SOCK_STREAM, STCP_PROTO_UDP);
```

## Protocol numbers

```c
#define STCP_PROTO_DEFAULT 0
#define STCP_PROTO_TCP     253
#define STCP_PROTO_UDP     254
```

Protocol `0` defaults to the TCP carrier.

## Normal BSD address handling

No local or remote address is supplied to the module.

Applications use ordinary operations:

```c
fd = socket(AF_STCP, SOCK_STREAM, STCP_PROTO_TCP);

bind(fd, ...);
listen(fd, ...);
accept(fd, ...);
```

or:

```c
fd = socket(AF_STCP, SOCK_STREAM, STCP_PROTO_UDP);

bind(fd, ...);
connect(fd, ...);
```

The STCP socket owns its own kernel TCP or UDP carrier socket.

## Reliability

TCP carrier:

- STCP framing and encryption remain active.
- STCP retransmission is disabled.
- TCP provides reliable ordered delivery.

UDP carrier:

- STCP ACKs remain active.
- send window and retransmission remain active.
- duplicate suppression remains active.
- out-of-order buffering remains active.

## Current UDP server limitation

UDP `listen()` and `accept()` still return `-EOPNOTSUPP`. The next phase adds
connection IDs and peer demultiplexing so one bound UDP socket can accept
multiple STCP sessions.

## Quick protocol-selection test

```bash
cc -O2 -Wall testing/stcp_protocol_select.c \
    -o testing/stcp_protocol_select

./testing/stcp_protocol_select
```

Expected:

```text
created STCP/TCP fd=... and STCP/UDP fd=...
```

## Integration

This archive is a focused integration package. Replace/adapt the supplied
`stcp_carrier.c`, protocol-number `stcp_create()` logic, Rust carrier file, and
the listed integration points in your current working tree.


## Fixed integration details

This revision fixes the missing per-context carrier pointer.

Add to `ContextInner`:

```rust
pub carrier: usize,
```

Initialize it to zero in every constructor and expose it through
`stcp_rust_set_carrier()`.

All `carrier::transmit()` calls now use:

```rust
transmit(
    &shared,
    side,
    carrier_ptr,
    &frame,
    0,
)?;
```

The C socket creation and accept paths must call:

```c
stcp_rust_set_carrier(ssk->rust_ctx, ssk->carrier);
```

and:

```c
stcp_rust_set_carrier(child->rust_ctx, child->carrier);
```

## UDP listen/accept demultiplexing

The UDP carrier now uses a 64-bit `connection_id` in the STCP v2 header. A
single bound UDP listener socket receives all datagrams and routes them to
child Rust sessions by `(connection_id, peer IPv4, peer port)`.

- First `PublicKey` frame for a new connection creates a pending child session.
- `accept()` attaches a lightweight child carrier that shares the listener UDP
  socket through a reference count and stores its own peer address.
- Unknown connection IDs and packets from a different peer tuple are dropped.
- Existing sequence/ACK logic provides duplicate suppression and retransmission.
- Child release unregisters its demux entry before the Rust context is freed.

Test manually:

```bash
cc -O2 -Wall testing/stcp_udp_test.c -o testing/stcp_udp_test
./testing/stcp_udp_test server 2 &
./testing/stcp_udp_test client first &
./testing/stcp_udp_test client second &
wait
```
