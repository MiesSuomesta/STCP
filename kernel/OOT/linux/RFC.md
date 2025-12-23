# STCP: Secure Transport Protocol with Built-In Cryptography
**Experimental Design RFC**

**Status:** Experimental  
**Audience:** Kernel networking / security community  
**Note:** This is a discussion document, not an Internet standard (not an IETF RFC).

---

## Abstract

STCP (Secure TCP) is an experimental transport-layer protocol that integrates encryption and peer authentication directly into the connection layer. Unlike traditional approaches that layer TLS over TCP, STCP treats security as a first-class property of the transport protocol itself.

This document describes the motivation, design goals, architecture, and use cases of STCP, and compares it with existing solutions such as TCP+TLS and QUIC. STCP is currently implemented as a Linux kernel module written in Rust and is intended for research, experimentation, and controlled environments.

---

## 1. Motivation

TCP was designed in an era where encryption and authentication were optional concerns. Modern systems compensate by layering TLS on top of TCP, introducing additional complexity, configuration burden, and duplicated state machines across user space and kernel space.

In many environments—particularly embedded systems, industrial networks, and controlled deployments—the complexity of managing TLS certificates, trust stores, negotiation, and user-space libraries can outweigh the benefits of flexibility.

STCP explores whether a simpler and safer abstraction can be achieved by integrating cryptography directly into the transport layer.

---

## 2. Problem Statement

The traditional TCP+TLS model introduces several challenges:

- Security is optional rather than inherent
- Operational complexity (certificates, trust stores, updates)
- Duplicate buffering and state machines across layers
- Heavy user-space dependency in constrained systems
- Difficult integration in kernel-space or low-level networking paths

STCP addresses these issues by embedding cryptographic mechanisms directly into the transport protocol.

---

## 3. Design Goals

- Secure-by-default connections (no plaintext fallback)
- Integrated encryption and authentication in the transport protocol
- Minimal configuration surface for targeted deployments
- Kernel-level implementation with an explicit state machine
- Suitability for embedded and constrained environments
- Clear failure modes and debuggability

---

## 4. Protocol Overview

STCP is a TCP-like, connection-oriented transport protocol with the following characteristics:

- Explicit cryptographic handshake at connection setup
- Encrypted payloads at the protocol level
- No separate TLS session layered on top
- Kernel-managed encryption and framing
- Userspace sees plaintext only after successful authentication

---

## 5. Handshake Model

STCP performs a handshake at connection establishment:

```
INIT -> EXCHANGE_KEYS -> DERIVE_KEYS -> AES_MODE
   \-> FAIL (teardown)
```

Handshake failures are fatal and result in immediate connection teardown.

---

## 6. Cryptographic Model

- Ephemeral key exchange (ECDH-based)
- Symmetric encryption for payload data (AES-GCM or equivalent)
- Per-session keys derived during handshake

All cryptographic operations are performed within the kernel module.

---

## 7. Wire Format

This section describes the on-the-wire framing used by STCP after the handshake has completed.
The format is intentionally simple and explicit to ease kernel-level parsing and debugging.

### 7.1 General Principles

- All multi-byte integer fields are encoded in **big-endian** (network byte order).
- There is **no plaintext data** after the handshake completes.
- Each STCP record represents exactly one encrypted payload unit.
- Record boundaries are explicit and length-prefixed.

---

### 7.2 STCP Record Layout (Encrypted Data)

After the connection enters encrypted mode, all application data is transmitted as STCP records
with the following structure:

```
0                   1                   2                   3
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                    Payload Length (64 bits)                   |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                  Initialization Vector (IV)                   |
|                          (fixed size)                         |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|                                                               |
|                 Encrypted Payload (variable)                  |
|                                                               |
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

---

### 7.3 Field Descriptions

#### Payload Length (64 bits)

- Unsigned 64-bit integer
- Specifies the length of the encrypted payload in bytes
- Does **not** include the length of the header or IV
- Allows large payloads while remaining explicit

#### Initialization Vector (IV)

- Fixed-size per-record IV
- Generated uniquely for each record
- Used by the AEAD cipher (e.g., AES-GCM)
- Transmitted in plaintext as part of the record header

#### Encrypted Payload

- Ciphertext produced by symmetric encryption
- Authenticated using AEAD
- Decryption failures are treated as fatal protocol errors

---

### 7.4 Authentication Tag

- The authentication tag produced by the AEAD cipher is appended to the encrypted payload
- Tag verification failure results in immediate connection teardown
- No unauthenticated data is exposed to userspace

---

### 7.5 Handshake Message Framing

Handshake messages are exchanged before encrypted mode is entered.
These messages are protocol-internal and are **not** exposed to applications.

Handshake messages are framed explicitly and processed only by the STCP state machine.
The exact wire format of handshake messages is considered internal and subject to change
while the protocol remains experimental.

---

### 7.6 Error Handling

- Any framing, length, or authentication error is treated as a fatal protocol violation
- There is no fallback to plaintext or partial recovery
- Connections are closed immediately on error

---

## 8. Comparison With Existing Solutions

### TCP + TLS
- Security layered above transport
- Complex configuration and certificate management
- Heavy user-space dependency

### QUIC
- User-space protocol over UDP
- Designed for Internet-scale traffic

### STCP
- Kernel-level transport
- Security integrated into protocol
- Intended for controlled environments

---

## 9. Use Cases

- Embedded and IoT systems
- Industrial or private networks
- Secure device-to-device communication
- Kernel-space networking paths

---

## 10. Non-Goals

- Replacing TCP globally
- Competing with TLS or QUIC for web traffic
- Acting as an Internet-wide standard

---

## 11. Security Considerations

STCP is experimental and has not undergone formal security review. Security claims should be considered preliminary.

---

## 12. Current Status & Limitations

- Experimental kernel module
- Not production-ready
- Protocol and APIs subject to change

---

## 13. Future Work

- Formal protocol specification
- Embedded/RTOS implementations

---

## 14. Discussion & Feedback Requested

Feedback is welcome on design trade-offs, wire format decisions, use cases, and kernel integration concerns.

---

**Repository:** https://github.com/MiesSuomesta/STCP/
**Contact:** lja@lja.fi
