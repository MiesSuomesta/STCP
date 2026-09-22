STCPv2 SAT compact wire overlay
Baseline: module-08092026_114423.tar.gz

Changes
=======

Active frame header:
  old: 40 bytes
  new: 17 bytes
  saving: 23 bytes/frame (57.5%)

Legacy packet header:
  old: 16 bytes
  new: 5 bytes
  saving: 11 bytes/packet (68.75%)

17-byte frame layout
====================
  byte 0      descriptor
              bits 0..3 = packet type
              bits 4..5 = STCP version (2)
              bits 6..7 = flags
  bytes 1..4  payload length, u32 BE
  bytes 5..8  sequence, u32 BE
  bytes 9..12 acknowledgment, u32 BE
  bytes 13..16 connection ID, u32 BE

Safety / compatibility notes
============================
- Internal sequence and acknowledgment state remains u64.
- Header::with_numbers() rejects values > u32::MAX instead of truncating.
- Connection IDs are already allocated by AtomicU32 in session.rs.
- Payload length remains u32 and still supports current 64 MiB max.
- The separate 8-byte AEAD nonce is NOT removed in this overlay.
- This is a wire-protocol break: both peers must use the new format.
- The ASCII STCP magic is removed from the wire. TCP parser validation now
  relies on descriptor version/type plus payload-length validity.
- Existing 16-bit frame flags were unused; compact wire format provides 2 bits.

Files replaced
==============
rust/src/frame.rs
rust/src/packet.rs
r/rust/src/frame.rs
r/rust/src/packet.rs
docs/specification/STCPv2-wire.md
r/docs/specification/STCPv2-wire.md

Apply
=====
From kernel/module root:
  tar -xzf stcp-sat-compact-wire-overlay-08092026_1150.tar.gz

Then rebuild/reinstall both endpoints before interoperability testing.

Validation performed here
=========================
- Searched source mirrors for residual STCP_MAGIC references: none.
- Searched source mirrors for old 16/40-byte header constants: none.
- Reviewed connection ID allocator: AtomicU32.
- cargo/rustc were not available in the artifact environment, so compile and
  runtime tests must be run in the STCP kernel build environment.
