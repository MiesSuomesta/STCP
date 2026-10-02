# Raspberry -> Raspberry STCP-TCP handshake/data coalescing fix

Postmortem `stcp-postmortem-20260823-075056.zip` showed that the TCP carrier,
local listener lookup, kernel accept, Rust child selection and cryptographic
handshake all progressed correctly.  The client reached Ready and immediately
sent its first DATA frame.

On the accepted Raspberry child, TCP coalesced the peer HandshakeDone and first
DATA into one receive batch. `process_handshake_frames()` consumed HandshakeDone
but continued its loop while the socket was still in Handshake state.  The next
DATA frame was therefore interpreted as an illegal handshake packet and Rust
returned `-EPROTO` (`-71`).  accept() then timed out.

The fix treats HandshakeDone as a parser boundary: consume it, stop the
handshake parser, transition to Ready, then let `progress_receive()` invoke the
normal application-data parser for bytes already queued behind it.

Debug event 253 (`HS-DONE-BOUNDARY`) records this boundary and the number of
bytes still queued after HandshakeDone.
