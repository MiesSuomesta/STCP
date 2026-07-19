# Segmented RX queue fix

The carrier previously appended all received bytes into a single
`VecDeque<u8>`. With pipelined traffic this grew to several megabytes and
forced Rust's allocator to request a large physically contiguous `kmalloc`
allocation.

The new `ByteQueue` stores data in independent 64 KiB chunks:

- no multi-megabyte contiguous allocation
- no full-buffer reallocation and copy when the queue grows
- wire and application RX queues are both segmented
- reads consume chunks incrementally
- header parsing uses `peek_prefix()`

The global Rust allocator now uses `kvmalloc()`/`kvfree()` as a safety fallback,
but the hot RX queues no longer rely on large single allocations.

The default pipeline is reduced to 8 for KASAN correctness testing. Increase it
gradually on a non-KASAN release kernel after validating memory behavior.
