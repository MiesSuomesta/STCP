# STCP Security & Handshake Analysis

## Overview

This document answers frequently asked security questions regarding **STCP (Secure TCP)**, with a particular focus on:

- Attack surfaces specific to STCP
- Handshake CPU cost
- Denial-of-Service (DoS) resistance
- Client vs server computational asymmetry

These questions are commonly raised by kernel and networking maintainers, and are addressed here in a technically precise manner.

---

## 1. Are there attacks specific to STCP security?

**Short answer:**  
STCP does **not introduce new classes of attacks** beyond those already known from authenticated key-exchange protocols (e.g. TLS, Noise). In several areas, STCP *reduces* the attack surface.

### 1.1 Man-in-the-Middle (MITM)

STCP provides **confidentiality and integrity**, but **MITM resistance depends on peer authentication policy**.

- STCP uses ECDH-based key exchange.
- Session keys are derived securely from the shared secret.
- **If no peer authentication is configured**, an active MITM attacker can:
  - Establish two independent STCP sessions
  - Transparently relay traffic between client and server

This behavior is equivalent to:
- TLS without certificate validation
- SSH with Trust-On-First-Use (TOFU) on first connection

### MITM prevention mechanisms

MITM attacks are prevented **when at least one of the following is enabled**:

- Public key pinning
- Trust-On-First-Use (TOFU)
- Pre-shared public keys
- External identity verification (certificates, policy hooks)

When authentication is enabled, a MITM attacker **cannot complete the handshake** without possession of the correct private key.

**Result:**  
- MITM is *possible* without authentication  
- MITM is *cryptographically prevented* once authentication is enforced

### 1.2 Replay Attacks

- Each handshake uses:
  - Ephemeral public keys
  - Session-specific randomness
- Session keys are never reused.
- Replayed handshake messages do not match current session state and are rejected.

**Result:** Replay attacks are ineffective.

---

### 1.3 Downgrade Attacks

- STCP does **not** allow silent cipher or protocol downgrades.
- Cipher selection is fixed per policy/build.
- Any mismatch results in immediate connection abort.

**Result:** Downgrade attacks are structurally prevented.

---

### 1.4 State Exhaustion Attacks

- The server keeps **minimal state** prior to handshake validation.
- No heavy allocations are performed until authentication succeeds.

**Result:** State exhaustion is mitigated by design.

---

## 2. How CPU-heavy is the STCP handshake?

The STCP handshake is **lightweight** and comparable to modern TLS handshakes.

### Typical cryptographic cost

| Operation | Approximate cost |
|--------|------------------|
| ECDH scalar multiplication (X25519 / secp256r1) | 50–200 µs |
| AES-GCM key setup | negligible |
| Total cryptographic cost | < 1 ms |

These values are **equal to or lower than TLS 1.3** handshake costs.

---

## 3. Does the client do heavy maths before the server?

**Yes — intentionally.**

STCP is explicitly designed to be **client-expensive and server-cheap** in the early handshake phase.

### Handshake asymmetry (anti-DoS)

#### Step 1: Client → Server
- Client generates:
  - Ephemeral keypair
  - Initial handshake message
- Client performs the **first expensive cryptographic operation**.

#### Step 2: Server early handling
- Server:
  - Validates message format
  - Performs minimal checks
  - Does not allocate heavy state
  - Does not commit expensive crypto yet

#### Step 3: Authentication success
- Only after validation:
  - Server performs its ECDH
  - Session transitions to AES-encrypted mode

**Key property:**  
A malicious client cannot force the server to perform expensive computation by stalling the handshake.


## 4. Denial-of-Service (DoS) Considerations

STCP is resistant to CPU-based and handshake-stalling DoS attacks:

- Half-open handshakes are cheap to discard
- Server CPU cost is delayed until client commitment
- Listening sockets are protected against cryptographic abuse

This makes STCP well-suited for kernel-space operation.

## 5. Comparison: TCP + TLS vs STCP

| Aspect | TCP + TLS | STCP |
|------|-----------|------|
| Crypto layer | User space | Kernel space |
| Early authentication | No | Yes |
| Server-side DoS exposure | Higher | Lower |
| Half-open handshake cost | Medium–High | Very Low |
| Per-packet overhead | Higher | Lower |


## 6. Summary

> STCP intentionally front-loads cryptographic cost to the client and
> minimizes server-side work prior to authentication.
>
> The handshake is lightweight (< 1 ms), designed to prevent CPU-exhaustion
> DoS scenarios where a malicious peer attempts to stall the handshake mid-way.
>
> From a security perspective, STCP introduces no new attack surface
> compared to existing authenticated key-exchange protocols, while
> reducing kernel exposure to unauthenticated traffic.


## Status

This document is intended for:
- RFC drafts
- LKML / netdev discussions
- Security review and threat analysis
