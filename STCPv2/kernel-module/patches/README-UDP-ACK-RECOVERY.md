# STCP/UDP cumulative ACK recovery fix

This revision fixes a large-message UDP stall where normal transfers completed
for a while and then one `recv()` waited until the application timeout.

The receiver now:

- re-sends a forced ACK even when the same cumulative ACK number was sent before;
- ACKs the highest contiguous sequence when a duplicate data frame arrives;
- sends a duplicate cumulative ACK immediately when an out-of-order frame exposes a gap.

The key detail is that an ACK already *sent* is not necessarily an ACK received
by the peer. Suppressing a forced duplicate ACK left the sender's pending frame
window occupied indefinitely after ACK loss.

Both `x86-kernel-module` and `raspberry-kernel-module` build the shared
`common-rust` crate, so rebuilding each module picks up the same fix.
