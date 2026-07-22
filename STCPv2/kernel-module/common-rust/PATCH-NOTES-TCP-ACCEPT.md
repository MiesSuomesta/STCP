# TCP carrier accept deadlock fix

The stream server used to wait for a child in the Rust `accept_queue` before
the C adapter called `kernel_accept()`. A remote TCP child cannot exist before
that carrier socket is accepted, so both sides waited forever and the client
handshake timed out.

For blocking TCP accepts, `session::accept()` now creates a provisional
server-side Rust context when the listener has a platform carrier. The existing
C flow then:

1. receives the Rust child context,
2. calls blocking `kernel_accept()`,
3. attaches the accepted carrier,
4. starts the receiver thread,
5. starts the server handshake.

Nonblocking accepts still return `-EAGAIN`; supporting remote nonblocking
accept cleanly requires a carrier-readiness callback or accept worker.
