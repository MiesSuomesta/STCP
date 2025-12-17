# STCP

STCP (Secure Transport Channel Protocol) is a lightweight, kernel-friendly  
encrypted transport protocol designed to provide:

- Authenticated encryption by default  
- Zero runtime configuration for applications  
- Simple integration path for kernel and userspace implementations  
- Compatibility with existing TCP-based infrastructure (when used as a wrapper)

This document describes the on-the-wire framing and cryptographic model used  
by the current STCP implementation.

> **Note:** This is an RFC-level specification for review and experimentation.  
> Wire details may be adjusted based on feedback from the networking and  
> Rust-for-Linux communities.

---

## 1. High-Level Overview

STCP provides an encrypted, authenticated byte-stream between two endpoints.

**Key properties:**

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

- **X25519** (or equivalent modern ECDH) for ephemeral key exchange  
- **HKDF-SHA256** for key derivation

**Outline:**

1. Client and server exchange ephemeral public keys.  
2. Both sides compute a shared secret via ECDH.  
3. A session key is derived:

   session_key = HKDF-SHA256(
       input_key_material = ecdh_shared_secret,
       salt               = handshake_nonce_or_random,
       info               = "STCP-AES-256-GCM v1"
   )

4. The resulting `session_key` (32 bytes) is used as the AES-256-GCM key.

### 2.2. Authentication Model

STCP does **not** hardcode a single trust model.

Allowed models include (implementation / deployment choice):

- Pre-shared key (PSK)
- Pinned public keys / embedded root key
- Out-of-band verification of peer identity

The important part for the kernel/RFC context:  
the handshake must result in an authenticated session key,  
or the connection is aborted.

If authentication fails → **tear down connection** (no fallback to plaintext).

---

## 3. Record Layer & Packet Format

Once the session key is established, all application data is sent as  
STCP records.

Each record is independently encrypted and authenticated with AES-256-GCM.

### 3.1. Nonce / IV

- AES-GCM is used with a **96-bit (12-byte) nonce**.  
- Nonces **must never repeat** for a given `(session_key)`.  
- Recommended construction (example):

  nonce[0..3]  = random per-connection prefix  
  nonce[4..11] = monotonically increasing counter per record

  The exact construction is implementation-defined as long as:  
  it is unique per record, per session key.

### 3.2. Wire Format

Each encrypted record has the following framing:

+-------------------------+--------------------+-------------------+------------------+  
| Length (8 bytes, BE)    | Nonce (12 bytes)   | Ciphertext (N B)  | Auth Tag (16 B)  |  
+-------------------------+--------------------+-------------------+------------------+

**Field details:**

- **Length**  
  - 64-bit unsigned, big-endian.  
  - Length of `(Nonce + Ciphertext + Tag)` in bytes.  
  - Used for framing on top of a stream transport.  

- **Nonce**  
  - 12-byte AES-GCM nonce used for this record.  

- **Ciphertext**  
  - Encrypted application data.  

- **Auth Tag**  
  - 16-byte GCM authentication tag.  

**Decryption process:**

1. Read 8-byte `Length`.  
2. Read `Length` bytes into buffer.  
3. Split into `Nonce (12)`, `Ciphertext`, `Tag (16)`.  
4. Run AES-256-GCM with session_key, Nonce, and optional AAD.  
5. On failure: treat as fatal error and close connection.  

No plaintext application bytes are transmitted outside this framing  
after the handshake is complete.

---

## 4. Error Handling

- Any authentication failure (GCM tag mismatch) MUST result in immediate  
  termination of the connection.  
- Unexpected record sizes or invalid lengths MUST be treated as protocol  
  errors; implementations SHOULD close the connection.  
- Implementations MAY implement timeouts and limits  
  (e.g. maximum record size).

---

## 5. Comparison to TLS (Non-Marketing)

STCP is not a replacement for TLS in all scenarios,  
but it explores a different set of tradeoffs:

- Tight integration with kernel and system-level code.  
- Simpler deployment for controlled environments  
  (no global CA infrastructure required).  
- Explicit, minimal feature set:  
  a single strong AEAD, one handshake pattern, no legacy modes.

### Key differences

Aspect | TLS | STCP  
-------|-----|------  
Handshake | Full negotiation (X.509, ALPN, cipher suites) | Fixed curve (X25519) + HKDF  
AEAD | Configurable | AES-256-GCM only  
Key management | PKI-based | Ephemeral + optional PSK  
Extensibility | High | Minimal  
Kernel friendliness | Low | Native integration (Rust + crypto API)

Security guarantees of STCP are only as strong as:

- the chosen authentication model,  
- the correctness of the handshake implementation,  
- and the enforcement of nonce uniqueness and AEAD usage.

---

## 6. Implementation Notes (Kernel Context)

For a kernel-space implementation (e.g. Rust-for-Linux):

- Use the in-kernel crypto API: gcm(aes) AEAD via `crypto_aead` interface in C.  
- Expose a small C wrapper used by Rust code:  
  - init/free AEAD context  
  - encrypt/decrypt one record  
- Keep Rust side responsible for:  
  - handshake state machine  
  - record framing  
  - error handling  
- Keep the wire format stable and documented.

---

## 7. Status

The current implementation is experimental and intended for:

- review by Rust-for-Linux and netdev maintainers,  
- validation of Rust as a first-class language for implementing  
  a real-world secure transport inside the kernel.

Feedback on the protocol design and kernel integration is explicitly welcome.

---

## 8. Example Test Vector (for validation)

Example vector (AES-256-GCM):

Field | Hex value  
------|------------  
Key | 000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f  
Nonce | 00112233445566778899aabb  
Plaintext | "STCP test message"  
Ciphertext + Tag | 9b8b4d8e2a7c937112b6769b08c2dbe0e034f19d49dfc39f7f585b5a3bb048fa

Use this vector to verify cross-compatibility between  
kernel-space and userspace implementations.

## Notes:

  [11.11.2025, 18:14]  Legacy code dropped from sources under kernel-module/kmod/
