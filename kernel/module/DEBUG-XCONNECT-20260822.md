# STCP cross-host connect crash instrumentation/fix (2026-08-22)

Scope: TCP cross-host connect path only.  This overlay is based on
`module-22082026_171417.zip`.

Changed files:

- `src/stcp_ops.c`
  - adds `stcp-xconnect: C01..C11` checkpoints around carrier connect, Rust
    connect, receiver startup, handshake start and connect wait.
- `src/stcp_carrier.c`
  - reduces the TCP RX scratch buffer from 8 MiB to 256 KiB. TCP is a byte
    stream and the Rust ByteQueue already handles split frames, so a full STCP
    frame does not need to fit into one kernel_recvmsg buffer.
  - changes RX startup from `kthread_run()` to create -> publish -> unlock ->
    wake. This makes carrier->receiver publication deterministic before the RX
    task can execute.
  - snapshots the TCP socket under lifecycle_lock and pins it with
    `active_sends` for the whole kernel_sendmsg loop.
  - removes the unsynchronised TCP `connected` pre-check; the TCP state/socket
    check is now made under lifecycle_lock.
  - adds `R*`, `RX*`, and `TX*` checkpoints around receiver creation,
    allocation, recvmsg and sendmsg.
- `src/stcp_memory.c`
  - enables names for Rust handshake/RX debug events 300..313.
- `rust/src/session.rs`
  - adds handshake/frame-send checkpoints.
- `rust/src/carrier.rs`
  - adds RX enqueue/progress checkpoints.

First postmortem grep:

    grep -E 'stcp-xconnect:|stcp-demux: (HS-|FRAME-|CARRIER-TX|RX-)' \
        host/journal-kernel-prev.log

The last emitted checkpoint should identify whether a freeze is in receiver
creation/allocation, Rust handshake state, kernel_sendmsg, the peer RX callback,
or the wait-for-ready phase.

Notes:

- The extra `pr_emerg()` logging is intentionally noisy for crash capture and
  should be removed or demoted after the fault is isolated.
- This environment did not contain Cargo/kernel build headers, so only source
  transformation/integrity checks were performed here; compile the overlay in
  the normal STCP kernel build environment before deployment.
