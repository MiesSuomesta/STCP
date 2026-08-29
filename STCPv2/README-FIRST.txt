STCPv2 native libp2p Noise XX empty-prologue fix
=================================================

Apply from the STCPv2 repository root:

  cd ~/STCP/STCPv2
  unzip -o /path/to/stcp-noise-empty-prologue-fix-overlay.zip

Changed file:
  kernel/module/rust/src/p2p/noise.rs

Fix:
  Noise initialization now performs MixHash(empty prologue), exactly as the
  Noise handshake initialization requires and as rust-libp2p/snow does for
  noise::Config::new().

Why this matches the observed failure:
  - chaining key (ck) was already correct
  - ee / MixKey produced the expected cipher key
  - handshake hash (h), used as ChaChaPoly AAD, was missing MixHash("")
  - therefore XX message 2 responder static decrypt failed with -EBADMSG

No transport, STCP framing, stack sizing, ChaCha implementation, or nonce
format is changed.
