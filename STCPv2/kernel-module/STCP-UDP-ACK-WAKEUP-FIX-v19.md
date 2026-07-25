# STCP UDP ACK wakeup fix v19

## Root cause

The C `stcp_sendmsg()` path blocks on `ssk->recv_wq` while the reliable UDP
flight window is full. Incoming ACK frames were parsed immediately in Rust and
removed entries from `pending_frames`, but `queue_to_context()` only woke the
socket owner when application data became readable or the handshake became
Ready.

ACK-only traffic therefore changed `can_send()` from false to true without
waking the sleeping sender. The sender resumed only when the fixed
`STCP_SEND_READY_TIMEOUT_MS` timeout expired after 30 seconds. This exactly
matches the repeated 30.1-32 second benchmark tail.

## Fix

`common-rust/src/carrier.rs` now identifies UDP control frames that can change
TX state (`Ack`, `Nack`, `Pong`, `Reset`, and `Close`) and wakes the child
socket's `recv_wq` after successful parsing. This wakes blocking `sendmsg()`
right after ACK progress while preserving the existing data-ready and
handshake wakeups.

No wire format, send window, ACK cadence, demultiplexing, or application receive
semantics were changed.
