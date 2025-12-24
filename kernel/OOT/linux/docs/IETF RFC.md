# STCP: A Secure Transport Protocol with Integrated Cryptographic Handshake

## Abstract

This document specifies STCP (Secure Transport Control Protocol), a transport-layer
protocol that integrates mandatory encryption and authentication directly into the
transport layer. Unlike TCP combined with external security mechanisms such as TLS,
STCP performs cryptographic negotiation as an intrinsic part of connection establishment.
The protocol is designed to operate in both kernel-space and user-space environments,
with deterministic handshake behavior and no plaintext fallback. STCP aims to reduce
configuration complexity, eliminate insecure deployment modes, and provide a secure
transport abstraction suitable for modern systems.

This document defines the STCP wire format, cryptographic handshake, state machine,
and security considerations. STCP is not intended to replace TLS, nor is it wire-compatible
with TCP.

---

## 1. Introduction

The Transmission Control Protocol (TCP) was designed at a time when network security
was not an inherent requirement. As a result, modern TCP-based applications rely on
additional security layers, most notably Transport Layer Security (TLS), to provide
confidentiality and authentication.

While TLS has proven effective, its separation from the transport layer introduces
operational complexity, configuration errors, and inconsistent security guarantees.
In particular, applications may inadvertently permit plaintext communication, misconfigure
certificate handling, or expose sensitive data prior to handshake completion.

STCP addresses these issues by integrating cryptographic handshake and encryption
directly into the transport protocol itself. Encryption is mandatory, occurs before
any application data is transmitted, and does not require application-layer involvement.

---

## 2. Design Goals and Non-Goals

### 2.1 Design Goals

STCP is designed with the following goals:

- Mandatory encryption for all connections
- No plaintext or insecure fallback modes
- Deterministic handshake and state transitions
- No application-layer cryptographic logic
- Suitability for kernel-space implementation
- Compatibility with user-space deployment
- Minimal configuration requirements

### 2.2 Non-Goals

STCP explicitly does not aim to:

- Replace TLS as an application-layer security protocol
- Provide wire compatibility with TCP
- Optimize for legacy middleboxes or deep packet inspection
- Provide application identity or certificate infrastructure

---

## 3. Architecture Overview

An STCP connection consists of:

- A client endpoint
- A server endpoint
- A cryptographic handshake phase
- A fully encrypted transport phase

The protocol enforces a strict state machine in which no application data may be
transmitted until the cryptographic handshake has completed successfully.

All cryptographic operations are performed within the STCP layer, independent of
the application using the connection.

---

## 4. Wire Protocol

### 4.1 Byte Order

All multi-byte fields in STCP are encoded in network byte order (big-endian).

### 4.2 Record Format

Each STCP record consists of the following components:

