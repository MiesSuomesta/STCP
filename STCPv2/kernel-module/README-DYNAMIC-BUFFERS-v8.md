# Dynamic benchmark buffers v8

The Zephyr benchmark allocates streaming buffers at runtime with `k_malloc()`.

- upload: one `chunk_size` TX buffer
- download: one `chunk_size` RX buffer
- full duplex: separate TX and RX buffers, total `2 * chunk_size`
- buffers are freed after success and error paths
- maximum runtime chunk is controlled by `CONFIG_BENCH_MAX_CHUNK_SIZE`

Default configuration supports 64 KiB chunks:

```ini
CONFIG_BENCH_MAX_CHUNK_SIZE=65536
CONFIG_HEAP_MEM_POOL_SIZE=163840
```

Runtime shell example:

```text
stcp config total 4194304
stcp config chunk 65536
stcp bench upload
stcp bench download
stcp bench full
```

For full duplex with 64 KiB chunks, reserve at least 128 KiB plus heap allocator and application overhead.
