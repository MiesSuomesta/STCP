# V6.11: deterministic TCP handshake receiver startup

The TCP carrier receiver used to start inside `stcp_carrier_connect()` and
`stcp_carrier_accept()`. On the accepted side this allowed already-buffered
PublicKey bytes to reach Rust before the child context had its carrier and
owner attached. `progress_handshake()` could then transmit HandshakeDone via
the carrier==0 fallback queue, mixing the in-memory and TCP paths and leaving
one or both endpoints permanently outside Ready.

Changes:

- `stcp_carrier_connect()` establishes TCP but no longer starts RX.
- `stcp_carrier_accept()` creates the accepted carrier but no longer starts RX.
- New `stcp_carrier_start_receiver_thread()` starts the receiver explicitly.
- Client order: TCP connect -> Rust connect -> PublicKey -> start RX.
- Server order: TCP accept -> attach Rust carrier/owner -> PublicKey -> start RX.

This guarantees that every received TCP byte is processed against a fully
initialized Rust context and every handshake response uses the TCP carrier.
