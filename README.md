# STCP — Secure Transport Communication Protocol

**A secure, transport-independent communication protocol designed for Linux, embedded systems, constrained networks and high-latency links.**

STCP is an experimental open-source networking protocol built around a simple idea:

> **Applications should communicate securely without having to care what physical or logical transport carries the data.**

STCP provides a common secure communication layer that can operate across different carriers and environments — from Linux systems and embedded devices to cellular, satellite and other constrained links.

The project is developed in Finland.

## Current generation

### STCPv6

STCPv6 is the current development generation of STCP.

The repository also includes the **STCP MiniSDK**, intended to make STCP integration possible without requiring applications to understand the internals of the kernel implementation or wire protocol.

STCP is no longer only a protocol experiment. The project includes working implementations, interoperability testing, automated test infrastructure and support work spanning Linux and embedded environments.

## Design goals

STCP follows two core rules:

**1. Every bit on the wire costs.**

Protocol overhead matters — especially on embedded, cellular, satellite and other constrained networks.

**2. KISS — Keep It Simple Stupid.**

Complexity should have to justify its existence.

The protocol is therefore designed around a small, understandable transport core rather than a large collection of optional negotiation layers and legacy compatibility mechanisms.

## What STCP provides

STCP is designed to provide:

- secure communication by default
- authenticated encryption
- transport independence
- low wire overhead
- a socket-oriented programming model
- Linux kernel integration
- embedded-system support
- communication over multiple carrier types
- support for high-latency and bandwidth-constrained environments
- a small integration surface through the MiniSDK

The long-term goal is simple:

**An application should communicate through STCP while STCP handles how the data reaches the other endpoint.**

## Architecture

Conceptually:

```text
Applications
     │
     ▼
┌─────────────────────────────┐
│            STCP             │
│                             │
│ Secure communication layer  │
└─────────────────────────────┘
     │        │        │
     ▼        ▼        ▼
    TCP      UDP      P2P
     │        │        │
     └────────┼────────┘
              ▼
       Physical transport
        Ethernet / LTE /
       satellite / others
```

The application-facing communication model is separated from the carrier used underneath it.

This allows STCP implementations to evolve or select transports without requiring the application protocol itself to be redesigned.

## Security

The current STCP implementation uses modern authenticated encryption.

STCPv6 builds on the project's AES-256-GCM based transport security work, with connection-specific cryptographic state and authenticated traffic.

Security is intended to be part of the transport rather than something every application has to bolt on independently.

STCP is still an experimental protocol and has **not been presented as a replacement for independently audited, standardized security protocols in every use case**.

Security review, interoperability testing and external analysis are welcome.

## Why another protocol?

TCP, UDP, TLS, QUIC and existing application protocols solve important problems extremely well.

STCP explores a somewhat different problem:

**Can applications use one small secure communication abstraction across very different underlying networks?**

This becomes particularly interesting when the endpoints are not simply two servers connected through a conventional low-latency Internet path.

Examples include:

- embedded devices
- IoT
- cellular links
- intermittent networks
- Raspberry Pi and edge systems
- machine-to-machine communication
- high-latency links
- satellite communication

In these environments, protocol overhead, reconnection behaviour, implementation size and transport flexibility can matter as much as raw throughput.

## STCP MiniSDK

The MiniSDK is the easiest entry point for developers who want to experiment with STCP.

Its purpose is to hide unnecessary protocol internals and provide a small application-facing interface.

You should not need to understand the entire STCP kernel implementation before writing your first STCP application.

See the MiniSDK directory and examples in this repository for the current API and build instructions.

## Repository layout

The repository preserves the development history of STCP.

```text
STCPv1/     Early development
STCPv2/     Protocol evolution
STCPv3/     Performance and architecture development
STCPv4/     Transport, crypto and interoperability work
STCPv5/     Further protocol and compression development
STCPv6/     Current generation

MiniSDK / SDK components
scripts/    Build, test and automation tooling
```

Older generations are retained intentionally. They document how the protocol evolved and make performance and design changes traceable.

**For new development, use STCPv6 and the current MiniSDK.**

## Platforms

STCP development and testing has included:

- Linux
- Raspberry Pi / ARM64
- Zephyr
- Nordic embedded hardware
- Ethernet
- TCP and UDP carrier paths
- embedded networking environments

Additional transports and platforms are part of ongoing development.

## Satellite and constrained networks

Satellite communication is one of the environments STCP is specifically interested in.

Satellite links make several protocol costs unusually visible:

- every transmitted byte has a cost
- latency can be large
- bandwidth may be limited
- connectivity may be intermittent
- unnecessary protocol chatter becomes expensive

STCP's low-overhead and carrier-independent architecture is intended to make it suitable for experimentation in these environments.

Real-world satellite validation is an important next step for the project.

## Project philosophy

STCP is not intended to become useful only if one company controls every application built on top of it.

The larger goal is an ecosystem.

Developers and companies are encouraged to experiment with STCP and build **their own software, devices, services and protocols on top of it**.

If STCP becomes useful infrastructure for something its original author never imagined, that is a success.

## Status

**STCPv6 and the MiniSDK are public and available for experimentation.**

The project should currently be considered:

- experimental
- under active development
- suitable for testing and research
- open to interoperability work
- open to external review and contributions

Production deployments should evaluate the implementation and security model for their own requirements.

## Getting started

Clone the repository:

```bash
git clone https://github.com/MiesSuomesta/STCP.git
cd STCP
```

Then start with **STCPv6** and the **MiniSDK**.

Platform-specific build, installation and test instructions live with their respective implementations so that commands remain synchronized with the code.

## Contributing

Contributions are welcome.

Useful contributions are not limited to code.

The project benefits from:

- protocol review
- security review
- interoperability testing
- new carrier implementations
- embedded ports
- benchmarks
- documentation
- bug reports
- real-world experiments

If you find something questionable in the protocol design, open an issue.

If you can break something, even better — report it.

Networking protocols improve through testing, criticism and independent implementations.

## Licensing

See:

- `LICENSE`
- `LICENSE.COMMERCIAL`

for the licensing terms applicable to the project.

## Origin

STCP is an independent Finnish networking project.

It was created from the belief that secure communication between applications should remain simple even when the networks underneath them are not.

The protocol, Linux implementation, embedded work, SDK and supporting test infrastructure have been developed as parts of the same project.

STCP is now public so that others can study it, test it, challenge it — and, hopefully, build things on top of it.

---

**STCPv6 + MiniSDK**

**Secure communication. Minimal overhead. Any transport.**
