# STCP

STCP (Secure Transport Channel Protocol) is a lightweight, kernel-friendly
encrypted transport protocol designed to provide:

- Authenticated encryption by default
- Zero runtime configuration for applications
- Simple integration path for kernel and userspace implementations
- Compatibility with existing TCP-based infrastructure (when used as a wrapper)

This document describes the on-the-wire framing and cryptographic model used
by the current STCP implementation.

> Note: This is an RFC-level specification for review and experimentation.
> Wire details may be adjusted based on feedback from the networking and
> Rust-for-Linux communities.

---

## 1. High-Level Overview

STCP provides an encrypted, authenticated byte-stream between two endpoints.

Key properties:

- Uses modern AEAD: **AES-256-GCM**
- Ephemeral key agreement per connection
- Per-record nonces (no nonce reuse)
- Authenticated encryption (integrity + authenticity)
- No X.509 / CA dependency by default

STCP can be implemented:

- as its own kernel protocol (e.g. `IPPROTO_STCP`), or
- as a secure framing layer over an existing TCP connection.

This spec focuses on the **framing and crypto**, independent of transport.

---

## 2. Handshake

Each STCP connection starts with a handshake that establishes a shared
session key for AES-256-GCM.

### 2.1. Key Agreement

The reference implementation uses:

- X25519 (or equivalent modern ECDH) for ephemeral key exchange
- HKDF-SHA256 for key derivation

Outline:

1. Client and server exchange ephemeral public keys.
2. Both sides compute a shared secret via ECDH.
3. A session key is derived:

   ```text
   session_key = HKDF-SHA256(
       input_key_material = ecdh_shared_secret,
       salt               = handshake_nonce_or_random,
       info               = "STCP-AES-256-GCM v1"
   )

