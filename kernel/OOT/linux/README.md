# STCP – Secure TCP (Linux Kernel Module)

STCP (Secure TCP) is a Linux kernel module implementing a transparent, encrypted,
TCP-compatible transport layer. STCP provides automatic key exchange and encryption
while preserving the standard POSIX socket API for userspace applications.

Applications using STCP do **not** need to implement cryptography or change their
send()/recv() logic.

## Key Features

- Kernel-space handshake and key exchange (ECDH)
- Kernel-space AES-GCM encryption
- Transparent send()/recv() API for userspace
- No userspace cryptography required
- TCP-compatible transport semantics
- Deterministic, non-busy-loop handshake state machine
- **Production-tested throughput up to ~245 MB/s (opt-level=1)**

## Status

**Status:** Production-ready, optimized.

**STCP release tag:** `stcp-release-18.12.2025`

**Golden tag:** `GOLDEN-2025-12-17_165539`

This tag marks the first version where handshake, encryption, and transparent
send/recv operation are fully implemented and verified.

## Performance
STCP has been stress-tested under steady load and sustains up to ~245 MB/s
of encrypted throughput with predictable sub-3 ms p99 latency.
See docs/PERFORMANCE.md for full results.

