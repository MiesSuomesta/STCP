STCPv2 ByteQueue retained-owned reuse correction
================================================

Apply in repository root:

  cd ~/STCP/STCPv2
  unzip -o stcp-zephyr-bytequeue-retained-owned-reuse-overlay.zip

Changed:
  kernel/module/rust/src/byte_queue.rs

Why:
  The previous retain-last-chunk fix intentionally keeps one empty chunk
  allocated when the queue drains. push_slice() already reuses that chunk's
  spare capacity correctly.

  The missing case was owned-buffer insertion:
    push_vec()
    push_vec_from()

  Those functions appended a new data chunk behind the retained empty front,
  creating this representation:

    [ empty retained chunk ][ real data chunk ]

  Some generic readers tolerate it, but it breaks the clean invariant that
  when ByteQueue.len != 0 the front chunk contains readable bytes and can
  interfere with front-chunk fast paths during handshake processing.

Fix:
  * push_vec()/push_vec_from() replace the sole retained empty ByteChunk in
    place with the owned incoming Vec. No allocation is added.
  * push_slice() is left alone because it already refills spare capacity.
  * discard()/read_into() get a defensive zero-progress guard so an empty
    front can never cause a spin if one is encountered.
  * retain-last-chunk behavior remains for wire/control traffic and therefore
    keeps the earlier NOMEM stage=9001 fix.

No protocol, crypto, handshake state machine, timeouts or socket semantics are
changed.
