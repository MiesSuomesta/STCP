# ARCHITECTURE

## Overview

STCP is an overlay over TCP.
TCP owns lifecycle, STCP owns payload.
Never swap sk->sk_prot.
Never attach to LISTEN sockets.

STCP operates as a TCP-compatible protocol implemented as a Linux kernel module.
It intercepts socket operations and transparently adds secure handshake and
encryption before data enters the TCP stack.

The design goal is to preserve the existing TCP programming model while ensuring
that all cryptographic operations occur exclusively in kernel space.

## Layered Architecture

```
Userspace Application
    |
    |  send() / recv()
    v
STCP socket operations (kernel)
    |
    |  plaintext <-> ciphertext
    v
Linux TCP stack
    |
    v
Network
```

Userspace applications interact with STCP exactly like a normal TCP socket.
Encryption, decryption, and key management are fully transparent.

## Responsibility Split: C vs Rust

### C (Kernel Glue Layer)

- Socket operation hooks (connect, accept, sendmsg, recvmsg, release)
- Workqueue scheduling and deduplication
- Lifecycle and shutdown safety
- Atomic initialization and exit gating
- Integration with the Linux networking stack

### Rust (Protocol Core)

- STCP protocol state machine
- Handshake logic
- RX/TX buffering
- Frame parsing and construction
- Cryptographic operations (ECDH, AES-GCM)

Rust code is isolated from kernel lifecycle complexity and focuses purely on
protocol correctness and cryptographic safety.

## Execution Model

- Socket events (connect, accept, data_ready) schedule a work item
- Each work execution performs a **single deterministic protocol step**
- No busy loops or uncontrolled retries are permitted
- Further progress depends on explicit socket events

This ensures predictable behavior and avoids race conditions or CPU spinning.
