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

## Status

**Status:** Production-ready core

**Golden tag:** `GOLDEN-2025-12-17_165539`

This tag marks the first version where handshake, encryption, and transparent
send/recv operation are fully implemented and verified.

