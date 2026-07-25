# STCP/UDP lost-ACK recovery fix

## Root cause

STCP data ACKs are cumulative, but ACK frames themselves are UDP datagrams and
may be lost. When a sender retransmitted an already received data frame, the
receiver called `queue_ack(..., force=true)`. The previous implementation still
returned early whenever `last_ack_sent >= sequence`, so it refused to resend the
lost ACK.

That leaves the sender's `pending_frames` window permanently occupied. The
sender retransmits until its retry limit is reached while the receiver keeps
silently discarding duplicates. At application level this appears as an
occasional `STCP receive timed out`, especially during long fragmented UDP
messages.

## Fix

`common-rust/src/session.rs` now:

- permits forced ACK retransmission even when the same cumulative ACK was sent
  before;
- ACKs the highest contiguous received sequence (`last_rx_sequence`), rather
  than downgrading the cumulative ACK to the duplicate frame's older sequence;
- keeps `last_ack_sent` monotonic.

Both `x86-kernel-module` and `raspberry-kernel-module` build the shared
`common-rust` crate, so the correction applies to both targets.

## Rebuild

Rebuild and reinstall both endpoint modules. A module rebuilt only on one side
is not sufficient for a bidirectional reliability test.

## Regression test

Run `benchmark/test-stcp-udp-ack-recovery.sh` after the server is started in
STCP/UDP mode. The default target is `192.168.1.199:19002`.
