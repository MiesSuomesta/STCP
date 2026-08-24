STCPv2 Zephyr TCP carrier write-all fix
=======================================

Fixes stream-carrier short-write handling without changing UDP semantics.

Why:
- Native TCP send may return a positive value smaller than the requested len.
- That is a successful partial write, not EIO.
- The STCP core expects stcp_carrier_send() to have delivered the whole wire
  frame when it reports success.
- The wrapper now loops over the unwritten tail for SOCK_STREAM.
- SOCK_DGRAM remains one atomic carrier call.
- EINTR is retried; zero write maps to EPIPE.
- Partial writes are logged so the W5500/Zephyr behavior is visible.

This file is based on the current platform baseline containing the software
RFC8439 ChaCha20-Poly1305 fallback.
