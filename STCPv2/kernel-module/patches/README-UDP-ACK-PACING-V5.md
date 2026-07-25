# STCP/UDP ACK pacing and oldest-frame recovery v5

This revision addresses the full-window retransmit loop seen under 8-16
concurrent 1 MiB sessions.

Changes:

- cumulative ACK is emitted for every committed UDP data frame;
- only the oldest unacknowledged frame is retransmitted per timer pass;
- cumulative ACK then releases the contiguous pending prefix;
- event 309 logs every ACK release (`arg0=released`, `arg1=remaining`);
- event 311 logs a duplicate/stale ACK while the send window is full.

The previous 16-frame retransmit burst could delay ACK processing and sustain a
full 128-frame window indefinitely even with no kernel/socket drops.
