# STCP UDP duplicate-ACK fast retransmit v11

This version is based directly on the user-provided latest kernel package.

## Fix

The cumulative ACK release loop already removed every pending frame with
`sequence <= acknowledgment`. The remaining stall was the recovery delay when
a send window stayed full and the receiver repeated the same cumulative ACK.

The sender now tracks duplicate cumulative ACKs. After three identical ACKs it
immediately retransmits the pending frame with sequence `ACK + 1` (or the first
pending sequence above the ACK), without waiting for the RTO timer.

## Diagnostics

- event 317: duplicate-ACK fast retransmit
  - arg0: retransmitted sequence
  - arg1: pending frame count

The existing timer-based bounded retransmit and selective NACK recovery remain
as fallbacks.
