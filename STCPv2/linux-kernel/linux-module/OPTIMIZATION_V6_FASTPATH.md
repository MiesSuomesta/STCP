# STCPv2 optimization V6 — queue/wakeup/lock fast path

## Applied changes

1. Fixed the kernel-next API use already present in this package:
   `kvfree_sensitive(buffer, buffer_size)`.
2. Removed duplicate RX wakeup from `stcp_receiver_thread()`.
   `stcp_rust_carrier_receive_from()` queues data and wakes the correct socket owner,
   so the unconditional C-side wake caused two wakeup traversals per receive.
3. Changed `ByteChunk` storage from `Box<[u8]>` to `Vec<u8>` and made
   `ByteQueue::push_slice()` fill the partially used tail chunk before allocating
   another chunk. This reduces allocator traffic and fragmented short chunks.
4. Replaced byte-at-a-time public-key extraction with `ByteQueue::read_into()`.
5. The DATA send loop now calls `carrier::transmit()` with the already cached
   carrier pointer instead of re-locking `ContextInner` through `send_frame()`.
6. RX decrypt, sequence-state update and zero-copy plaintext queue insertion now
   happen under one `ContextInner` lock acquisition instead of two.

## Expected effect

The largest benefit should appear with small/medium payload stress tests and many
clients, where scheduler wakeups, spinlock acquisitions and allocator calls
otherwise dominate actual encryption and copying.

## Validation limits

The archive was checked for ZIP integrity and source-level consistency. This
sandbox does not contain the package's Rust toolchain or matching Linux kernel
headers, so the actual external-module build must be run on the STCP development
host.
