# STCP cross-host TCP and UDP socket creation fix

Changes:

1. `src/stcp_proto.c`
   - Accept `SOCK_DGRAM` for protocol 254 (`STCP_PROTO_UDP`).
   - Reject invalid stream/datagram and protocol combinations with `EPROTOTYPE`.

2. `rust/src/session.rs`
   - Preserve the local in-kernel listener fast path when the target listener is
     present in the process-local registry.
   - When the real TCP carrier connected to a remote machine and no local
     listener exists, initialise an external client handshake instead of
     returning `ECONNREFUSED`.
   - Allocate a non-zero connection id so the remote accepted child can adopt it
     from the first PublicKey frame.

Expected impact:

- STCP-TCP Linux -> Raspberry and Raspberry -> Linux no longer fail solely
  because the remote listener is absent from the local Rust listener registry.
- `socket(AF_STCP, SOCK_DGRAM, 254)` reaches the existing UDP carrier path.
