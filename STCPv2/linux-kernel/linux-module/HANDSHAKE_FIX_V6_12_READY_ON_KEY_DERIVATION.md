# V6.12 handshake readiness fix

The stress result bundled with the submitted tree proves that TCP carrier setup
and accept work (`connections_ok=4`, `server_accepts=4`), but every first send
times out. The remaining gate was Rust `SocketState::Ready`.

Previously Ready required both:

1. successful peer public-key derivation, and
2. receipt of a separate `HandshakeDone` frame.

The second condition caused a readiness deadlock in the nonblocking poll path.
V6.12 enters Ready immediately after peer-key derivation and transmission of
the local HandshakeDone. Incoming HandshakeDone is still parsed and recorded,
but no longer blocks application I/O.
