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


## Adaptive retransmission timeout

See `ADAPTIVE_RTO.md`. Run `make LLVM=1 V=1 test-reliability` to exercise baseline, packet loss, delay, duplicate and reorder cases.

## Hallittu moduulin alasajo

Moduuli listaa aktiivisten STCP-socketien omistajaprosessit tiedostossa
`/proc/stcp/users`. Hallittu alasajo lähettää niille ensin `SIGTERM`-signaalin,
odottaa socketien sulkeutumista, käyttää tarvittaessa `SIGKILL`-signaalia ja
suorittaa lopuksi turvallisen `modprobe -r stcp` -komennon.

```sh
make module-stop
# alias:
make module-shutdown
make module-remove
```

Odotusaikoja voi säätää tarvittaessa:

```sh
TERM_WAIT=10 KILL_WAIT=5 make module-stop
```

## Rust datapath debug events

The patched build emits allocation-free numeric trace events through
`stcp_kernel_debug_event()`:

- 100/199: FFI recv enter/return
- 101/102: session recv enter/handshake progressed
- 110/111: fill_application_buffer enter/return
- 112: application queue state
- 120: wire parsing begins
- 121: ACK deferred outside wire lock
- 122: PONG deferred outside wire lock
- 123/124: deferred controls and extracted DATA frame counts
- 130-133: in-order DATA decrypt/store/ACK stages
- 140-143: control-frame parser lock/deferred-action stages

The lock order fix ensures that the carrier ByteQueue lock is never held while
updating `ctx.inner` or synchronously transmitting ACK/PONG frames. This removes
the queue-lock -> inner-lock / inner-lock -> carrier-send inversion.

## Parser deadlock hardening

This revision serializes handshake/control/data parsing per socket with
`parser_busy`. A complete wire frame is now extracted in two phases:
header inspection under the wire lock, allocation outside the lock, and a
short validated consume under the lock. Crypto, ACK processing, PONG
transmission, application buffering and debug printing run after releasing
the wire lock. This removes the suspicious allocation-under-spinlock,
wire-lock/inner-lock inversion and synchronous carrier callback paths.

Additional Rust debug events:

- 200-series: frame extraction and parser contention
- 210: extracted frame type and payload length
- 299: application parser completed

## 2026-07-19: release-parser fix

Fixed an infinite control-frame parser loop in release builds. `session.rs::remove_header()` previously called `ByteQueue::discard()` only inside `debug_assert_eq!`; release builds compile debug assertions out, so zero-payload frames such as HandshakeDone were never consumed. Header removal is now unconditional and returns a protocol error on a short discard. A bounded retry guard was also added to frame extraction to prevent any future queue race from creating an unbounded kernel busy loop.

## Handshake readiness race fix

Blocking `connect()` now waits until the Rust session reaches the connected/Ready
state before returning success. Nonblocking connects return `-EINPROGRESS` and
become writable through the existing poll waitqueue when the handshake completes.
Blocking `sendmsg()` also uses a bounded readiness wait instead of waiting forever.
This prevents the first 64-byte smoke-test send from racing ahead of handshake
completion and leaving the echo server blocked in `recv()`.

## 2026-07-19 handshake Ready race fix

The Rust session now enters `SocketState::Ready` only after both conditions are true:

- directional crypto keys have been derived, and
- the peer's `HandshakeDone` frame has been received.

Previously crypto readiness alone made `connect()` succeed, allowing the first DATA frame to race the accepted child's handshake completion. Debug events 250-252 expose the two readiness inputs and the final transition.
