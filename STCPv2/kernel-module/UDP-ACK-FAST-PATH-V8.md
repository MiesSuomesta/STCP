# STCP/UDP ACK fast path v8

This revision processes valid cumulative ACK control frames immediately while
extracting the wire-frame parser batch.

Previously ACKs were collected into `deferred_acks` and applied only after up
to 128 frames had been extracted. With 8-16 concurrent 1 MiB UDP sessions this
could leave a sender at `pending_frames == STCP_SEND_WINDOW` while additional
data frames were parsed, causing avoidable retransmit bursts and application
receive timeouts.

The ACK handler already removes every pending frame whose sequence is less than
or equal to the cumulative acknowledgment. V8 keeps that logic and moves its
invocation to the ACK extraction branch.

Debug event 123 now reports:

- arg0: ACK frames processed immediately in the parser pass
- arg1: deferred PING frames

Expected behavior:

- event 309 should appear closer to the corresponding 40-byte ACK receive
- full-window event 301 periods should shorten
- retransmit event 308 should reduce under concurrent load
