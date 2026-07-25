# STCP/UDP send-window partial-write fix

## Problem

Reliable UDP previously required the caller's complete Rust send chunk to fit in
`STCP_SEND_WINDOW`. With a 128-frame window and 64-frame C chunks, a state such
as `pending_frames=112` rejected the complete 64-frame request even though 16
frames were available. Under parallel large transfers this caused head-of-line
blocking, repeated `event=301`, and application receive timeouts.

## Fix

`common-rust/src/session.rs` now:

- reports writable when at least one reliable UDP frame fits;
- limits each `session::send()` call to the currently available frame capacity;
- returns the exact accepted byte count as a normal SOCK_STREAM partial write;
- emits debug event `310` when a write is shortened by send-window pressure.

The existing C `stcp_sendmsg()` wrapper already returns a positive partial byte
count and stops the current syscall. Normal userspace `sendall()`/write retry
semantics send the remainder without losing stream data.

## Expected diagnostic change

Old behaviour:

```
event=301 pending=112 requested_frames=64
```

New behaviour:

```
event=310 accepted_bytes=22400 pending=112
```

The accepted amount can vary as cumulative ACKs drain the window.

## Build and test

Rebuild and reload both x86 and Raspberry Pi modules, restart the STCP/UDP
server, then run:

```bash
python3 benchmark/benchmark_client.py \
  --mode stcp --transport udp \
  --host 192.168.1.199 --port 19002 \
  --clients 8 --payload 1048576 \
  --pipeline 1 --duration 30 --timeout 40
```

Then repeat with 16 clients.
