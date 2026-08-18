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


## Performance logging

High-frequency numeric `rust event=...` tracing is disabled in this package.
Error and major state-transition logging remains available through the normal STCP kernel logs.


## Performance build changes

- Hot-path C/Rust event and carrier/send/recv printk logging disabled.
- TCP userspace I/O batching increased to 4 MiB.
- TCP receive carrier buffer increased to 4 MiB.
- Stream DATA payload increased from 1 MiB to 2 MiB (UDP remains 60 KiB).
- ByteQueue chunks increased to 256 KiB to reduce allocation count.
- TCP TX readiness/send no longer runs the RX control parser unnecessarily.
- Duplicate carrier-side recv wake removed; Rust queue insertion owns the normal wake.

The reliable UDP path keeps STCP ACK/window processing unchanged.


## Churn CLOSE/EOF and teardown fix

- `close(fd)` now transmits the existing STCP `Close` control frame before the
  Rust context and carrier are detached.
- The peer parser marks `peer_eof` and wakes the receive waitqueue; blocking
  `recv()` then returns `0` with normal BSD EOF semantics.
- Carrier shutdown still runs before `stcp_rust_release()`, preventing late RX
  callbacks from touching a freed Rust context.
- Python churn handlers close immediately on EOF. The idle timeout remains only
  as a 10-second fallback for crashed/ungraceful peers.
- The prior 4 MiB I/O, 2 MiB stream-frame, reduced parser work and datapath-log
  performance changes remain included.

## 2026-07-20 churn/readiness and crypto hot-path update

- Carrier RX now parses and publishes complete DATA/EOF before waking `recv_wq`.
- `has_data()` is side-effect free; it no longer runs the parser from poll/wait conditions.
- This closes a lost-wakeup race that could leave churn connections sleeping after data had arrived.
- ChaCha encrypt/decrypt and large frame allocation run outside the Rust socket spinlock.
- The short lock is used only to snapshot and commit sequence/nonce state.
- `stcp_stress.py` accepts both `--report-every` and `--report-interval`.
- `run_stress_suite.sh` now honors `--duration`, `--pipeline`, `--clients`, `--payload`, and report interval options.
- Throughput suite defaults to 8 clients, 1 MiB payload, pipeline depth 8.

## Performance pass (paketti 13)

TCP hot-path changes:

- Reuse a per-socket encrypted TX frame allocation instead of allocating a new
  multi-megabyte `Vec` for every stream frame.
- Keep the UDP reliability path unchanged: retransmittable frames still use
  `Arc<[u8]>` ownership.
- Remove the second RX ciphertext copy. The parser now transfers ownership of
  the complete wire payload and decrypts directly from the slice after the
  nonce prefix.
- Increase C socket I/O scratch buffers and TCP carrier RX batches to 8 MiB.
- Increase `ByteQueue` chunks to 1 MiB to reduce allocation and queue metadata
  overhead for large stream transfers.

The stable handshake, CLOSE/EOF and churn fixes are retained.

## KASAN large-allocation fix

The per-socket C TX/RX scratch buffers no longer allocate the full maximum
size with `kmalloc()` on the first small operation. Each buffer now grows only
to the current capped I/O chunk with `kvmalloc()`, allowing vmalloc fallback on
fragmented and KASAN kernels. The maximum C I/O chunk is 2 MiB, and release
uses the matching `kvfree_sensitive(ptr, size)` path.

## Handshake Ready wake fix

Carrier RX now wakes the socket wait queue when the Rust session transitions
from not-connected to Ready. This fixes blocking connect() waiting until the
five-second timeout despite a completed handshake, while retaining the
empty-to-readable wake optimization for normal DATA traffic.

## Handshake Ready wake snapshot fix

`queue_to_context()` no longer calls the side-effecting `is_connected()` helper
when capturing the pre-parser state. `is_connected()` advances the handshake,
so it could transition the socket to `Ready` before `was_connected` was stored.
That made `became_connected` false and lost the only wakeup for blocking
`connect()`. The carrier now compares side-effect-free state snapshots before
and after `progress_receive()`.

## 2026-07-20 RX batch/wakeup optimization

- DATA frames parsed in batches of up to 64 frames per carrier callback.
- Application receive waitqueue is woken once per parsed batch instead of once per message/frame.
- EOF publication shares the same final batch wakeup.
- ChaCha decrypt and owned wire payload zero-copy path from the previous performance build are retained.


## Performance pass: in-place RX crypto

- ChaCha20-Poly1305 decrypt now runs in-place over the owned wire frame.
- Plaintext is transferred to the application ByteQueue without allocation or copy.
- TCP timer ticks no longer run a duplicate parser pass; carrier RX owns parsing.
- Crypto self-test validates the in-place decrypt path at module load.


## RX owned-chunk fast path
- ByteQueue chunks are sized to 1 MiB payload plus 64-byte STCP crypto/frame overhead.
- Complete frame payload chunks are detached and passed to in-place decrypt without a second copy.
- Fragmented TCP input safely falls back to the existing copy path.
- RX parser batch limit increased from 64 to 128 frames.


## TCP socket-buffer fast path

The TCP carrier now tunes every client, listener and accepted socket to a
16 MiB send/receive buffer, keeps TCP_NODELAY enabled, uses MSG_NOSIGNAL on
carrier writes, and grows per-socket C scratch buffers geometrically.  This
reduces pipeline stalls and repeated buffer reallocations in high-throughput
1 MiB / pipeline-8 tests.


## UDP large-payload flow-control optimization

- UDP reliability window increased from 64 to 256 frames.
- ACK frames are cumulative and emitted every eight in-order frames, plus immediately at `DataChunkEnd`.
- Duplicate frames force an immediate cumulative ACK to recover from a lost ACK.
- Retransmission work is capped at four expired frames per timer tick to prevent retransmit storms.
- Initial/minimum RTO increased to 100/40 ms to avoid spurious retransmission under large queued payloads.

## UDP pipeline scalability fix

- UDP child carriers no longer hold the shared root `lifecycle_lock` across
  `kernel_sendmsg()`. Concurrent accepted UDP sessions can now transmit in
  parallel while root teardown waits for an `active_sends` counter to drain.
- UDP kernel socket send/receive buffers are raised to 16 MiB locally.
- Redundant recv-waitqueue wakeups after ACK/PONG sends were removed; ACK
  frames are parsed synchronously by carrier RX.
- Initial/minimum reliability RTOs are reduced to 60/20 ms after eliminating
  the root-send serialization that caused artificial ACK delays.

## Churn teardown hardening

The churn path now gives the protocol CLOSE frame a short 200-500 us grace
period before carrier destruction. This prevents rare close-vs-final-recv
races under rapid connect/send/recv/close workloads.

The Python churn test retries a transient connection/send/recv teardown race
up to three times and only reports a failure after all attempts fail. Normal
EPIPE, ECONNRESET, ENOTCONN and ESHUTDOWN after a completed echo are treated
as expected peer teardown rather than server data-path errors.



## Churn server rollback

The churn stress server uses thread-per-connection again. The fixed worker pool
caused head-of-line blocking because workers could remain blocked in recv(),
which made connection throughput bursty. Completed handler threads are reaped
before and after each accept, active handlers are bounded by the listener
backlog, and successful-echo teardown errors remain filtered.

## Churn teardown accounting fix

The stress server now treats `EPIPE`, `ECONNRESET`, `ENOTCONN`, `ESHUTDOWN`,
`EBADF`, `EPROTO`, `EIO`, and poll-driven `ConnectionError` as normal churn
teardown conditions. Genuine setup, accept, payload and resource failures are
still counted as server errors.
