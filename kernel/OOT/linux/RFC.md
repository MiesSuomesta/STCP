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
- Userspace sees plaintext only after successful authentication (depending on integration model)

---

## 5. Handshake Model

STCP performs a handshake at connection establishment:

1. Exchange of public key material  
2. Derivation of a shared secret  
3. Transition to encrypted communication mode  
4. Protocol consistency checks; failures are fatal  

Handshake failures result in immediate connection teardown.

```
INIT -> EXCHANGE_KEYS -> DERIVE_KEYS -> AES_MODE
   \-> FAIL (teardown)
```

---

## 6. Cryptographic Model

- Ephemeral key exchange (ECDH-based)
- Symmetric encryption for payload data (AES-GCM or equivalent)
- Per-session keys derived during handshake

All cryptographic operations are performed within the kernel module.

---

## 7. Comparison With Existing Solutions

### TCP + TLS
- Security layered on top of transport
- Complex configuration and certificate management
- Heavy user-space dependency

### QUIC
- User-space protocol over UDP
- Designed for Internet-scale web traffic
- Requires congestion and reliability logic in user space

### STCP
- Kernel-level transport with integrated cryptography
- Minimal configuration surface
- Intended for controlled or specialized environments

---

## 8. Use Cases

- Embedded and IoT systems
- Industrial control or private networks
- Secure device-to-device communication
- Kernel-space networking paths where user-space TLS is impractical

---

## 9. Non-Goals

STCP does not aim to:
- Replace TCP globally
- Compete with TLS or QUIC for web traffic
- Provide backward compatibility with existing TCP stacks
- Act as an Internet-wide standard at this stage

---

## 10. Security Considerations

STCP has not undergone formal security review or cryptographic auditing. Security properties should be considered preliminary.

---

## 11. Current Status & Limitations

- Experimental Linux kernel module
- Not production-ready
- APIs and protocol subject to change
- Limited interoperability testing

---

## 12. Future Work

- Formal protocol specification
- Security review and threat model
- Cryptographic agility
- Embedded/RTOS implementations (e.g., Zephyr)

---

## 13. Discussion & Feedback Requested

Feedback is requested on:
- Motivation validity
- Design trade-offs
- Security assumptions
- Kernel integration concerns

---

**Repository:** https://github.com/MiesSuomesta/STCP/new/main/kernel/OOT/linux
**Contact:** lja@lja.fi

