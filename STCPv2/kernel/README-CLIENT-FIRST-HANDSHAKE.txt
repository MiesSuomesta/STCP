STCPv2 shared-core external TCP client-first handshake overlay
================================================================

Baseline
--------
kernel-25082026_085810.zip

Changed files
-------------
module/rust/src/state.rs
module/rust/src/session.rs

Why
---
The complete logs show that on the failing first Zephyr/W5500 boot the Linux
server successfully accepts TCP and immediately sends its 104-byte STCP
PublicKey frame, while the Zephyr side never reaches X25519 shared-secret
derivation. On a later boot the same path succeeds.

This overlay avoids sending the first STCP application payload from the
external TCP server immediately after TCP establishment:

  client TCP connect
  client -> PublicKey
  server receives PublicKey
  server -> PublicKey
  server -> HandshakeDone
  client -> HandshakeDone
  both -> Ready

Only external TCP server children use client-first initiation. The existing
internal paired Linux/Raspberry path is left unchanged.

Hardening
---------
PublicKey and HandshakeDone sends are now explicitly idempotent with:
  local_public_key_sent
  local_handshake_done_sent

This also prevents duplicate control-frame emission if progress_handshake()
is entered repeatedly.

Apply
-----
Extract over the tree containing module/rust/, rebuild the canonical shared
core, then rebuild/flash Zephyr and restart the Linux test server.

Expected first-boot result
--------------------------
The server should not emit its 104-byte PublicKey until after it has received
the Zephyr client's 104-byte PublicKey. The Zephyr first boot should then reach
"X25519 shared-secret complete" without requiring a second board/modem boot.
