STCPv2 shared-core Zephyr RX memory overlay
===========================================

Baseline
--------
kernel-25082026_085810.zip plus the previous client-first handshake overlay.

Changed files
-------------
module/rust/src/state.rs
module/rust/src/session.rs
module/rust/src/byte_queue.rs

Fixes
-----
1. Preserve the previous client-first external TCP handshake and make the
   missing Role import permanent.

2. SOCK_STREAM data becomes readable after every decrypted STCP frame.
   Previously rx_message_ready was set only on DataChunkEnd, so a 1 MiB
   send() had to accumulate completely in the Zephyr heap before recv()
   could drain a byte.

3. ByteQueue wire storage now uses fixed 16 KiB chunks reserved once.
   The old code repeatedly try_reserve_exact()'d the same tail chunk on
   each W5500/TCP receive, causing allocator churn and heap fragmentation.

4. Temporary RX parsing is capped at 8 frames per batch instead of 128,
   bounding short-lived ciphertext ownership to roughly a few tens of KiB.

Expected result
---------------
The recurring:
    stcp_v2_rx: carrier_receive rc=-12
should disappear or be drastically reduced, while application recv() drains
stream data continuously.

Protocol 254/datagram semantics are unchanged: DataChunkEnd still controls
message readiness there.
