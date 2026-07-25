# STCP/UDP connection-ID routing fix v7

The UDP listener previously routed frames to a child only when all four fields
matched: listener pointer, connection ID, peer IPv4 address and peer UDP port.
Under concurrent load, valid 40-byte ACK/control frames could reach the listener
with peer metadata that did not compare identically to the tuple stored during
child creation. Those frames were silently discarded, leaving that child's
128-frame reliability window permanently full.

V7 changes routing as follows:

- `(listener, connection_id)` is the authoritative child lookup key.
- Peer address/port mismatches no longer discard an otherwise matching frame.
- Event 312 reports a peer metadata mismatch (`arg0=received port`,
  `arg1=registered port`).
- Event 313 reports a non-PublicKey frame for an unknown connection ID.
- The session registry lock is still held while the raw child pointer is used,
  preserving the existing RX-vs-release lifetime protection.

This release retains the balanced ACK pacing, four-frame retransmit burst,
partial send-window writes, cumulative ACK recovery and shared UDP RX dispatcher
from v6.
