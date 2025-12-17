# HANDSHAKE

## Handshake Overview

STCP uses an explicit, deterministic handshake to establish a shared encryption
key before entering encrypted data mode.

The handshake is symmetric but role-dependent (client vs server).

## Client Handshake Flow

```
INIT
 ├─ Generate key pair
 ├─ Send public key (HS1)
 ├─ Receive server public key (HS2)
 ├─ Derive shared secret
 └─ Enter AES_READY
```

## Server Handshake Flow

```
INIT
 ├─ Generate key pair
 ├─ Receive client public key (HS1)
 ├─ Derive shared secret
 ├─ Send server public key (HS2)
 └─ Enter AES_READY
```

## State Machine Properties

- Exactly one handshake step per pump execution
- Progress only occurs when required data is available
- No busy loops or speculative execution
- Explicit failure states for protocol violations

## Handshake Invariants

- Both peers must derive the same shared secret
- Handshake messages are authenticated via protocol framing
- No application data is transmitted before AES_READY
- Any protocol violation results in immediate failure

## Transition to Encrypted Mode

Once the shared secret has been derived:

- AES-GCM context is initialized
- Socket enters `AES_READY` state
- sendmsg/recvmsg transparently encrypt and decrypt payloads

After this point, all data on the wire is encrypted.

