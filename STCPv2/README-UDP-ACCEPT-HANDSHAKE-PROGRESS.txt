STCPv2 UDP accepted-child handshake progress fix
================================================

Scope
-----
One source change only:
  kernel/module/rust/src/session.rs

Problem observed
----------------
Linux UDP listener creates an accepted child and reaches:

  A17 handshake-wait-exit ret=0 connected=0 terminal=0

The UDP child shares the listener RX path. The peer PublicKey / HandshakeDone
may already have been queued before userspace accept() calls start_handshake().
queue_to_context() normally calls progress_receive(), but try_parser_guard()
can legitimately defer a concurrent parser pass.

Fix
---
start_handshake() now:

  1. validates Handshake state/carrier
  2. sends the local PublicKey
  3. immediately calls progress_handshake()

This consumes any peer handshake frames that were already queued before the
kernel accept path sleeps waiting for Ready.

Safety
------
- No Linux C socket/carrier/accept changes.
- No wire format changes.
- No protocol number changes.
- TCP/253 behavior remains semantically unchanged: if no frame is queued,
  progress_handshake() is a no-op.
- UDP socket-type fix remains independent.

Expected effect
---------------
For the failing UDP accepted child, A16/A17 should either:
- observe connected=1 immediately/very quickly, or
- receive a wakeup when the peer HandshakeDone arrives.

After CoAP passes, run the full STCPv2 regression suite again.
