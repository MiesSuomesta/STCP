# STCP stream handshake recovery v2

Changes in this package:

- Replaced the one-shot connect/accept wait with a bounded 250 ms progress loop.
- Calls the Rust handshake parser during every progress check.
- Re-emits the idempotent PublicKey frame once per second while still in Handshake.
- Keeps the existing hard `connect_timeout_ms` deadline.
- Shuts down a failed client carrier and destroys failed accepted children cleanly.
- Adds rate-limited carrier TX/RX byte diagnostics to both x86 and Raspberry modules.

Expected diagnostic sequence on both hosts:

```
stcp: carrier TX ... bytes=<PublicKey frame size>
stcp: carrier RX ... bytes=<frame bytes>
stcp: connect/accept handshake retry ...
stcp: connect/accept handshake complete ...
```

Build and load the same source revision on both client and server hosts.
