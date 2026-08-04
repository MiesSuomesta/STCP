# Raspberry Pi STCP kernel module fixes

Applied changes:

1. `AF_STCP + SOCK_DGRAM + protocol 254` is accepted for STCP-UDP.
2. TCP and UDP protocol/socket type mismatches return `EPROTOTYPE`.
3. STCP-TCP connect keeps the local in-kernel listener fast path, but when the
   destination is remote it no longer fails solely because the remote listener
   is absent from the local Rust `LISTENERS` registry.
4. Remote STCP-TCP clients initialize handshake state and a non-zero connection
   id after the kernel TCP carrier has connected.

Build and test on the Raspberry kernel tree; this sandbox does not contain the
matching Raspberry kernel headers/toolchain.
