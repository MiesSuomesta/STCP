# STCP kernel architecture

The Rust implementation is now separated into stable layers:

```text
Linux BSD socket wrappers (C)
            |
            v
          ffi.rs
            |
            v
        session.rs
       /     |      \
      v      v       v
 frame.rs  crypto.rs carrier.rs
      \      |       /
             v
           state.rs
```

## Layer responsibilities

### `ffi.rs`
Owns only the C ABI. It validates raw pointers and converts Rust errors to Linux
negative errno values.

### `session.rs`
Owns socket state transitions and orchestration:

- bind/listen/connect/accept
- symmetric handshake
- send/receive flow
- shutdown/release
- nonce and replay checks

### `frame.rs`
Owns the STCP wire format:

- 16-byte header
- packet types
- header validation
- plaintext and encrypted frame encoding

### `crypto.rs` and `kdf.rs`
Own key generation, shared-secret handling, HKDF, directional session keys,
AEAD encryption/decryption, and key erasure.

### `carrier.rs`
Owns transport queues and wakeups. It is currently an in-kernel loopback
carrier. Replacing this file is the next step toward UDP/raw-IP transport.

### `state.rs`
Contains socket/session state and shared connection objects. It contains no
wire encoding or C ABI logic.

## Compatibility

The C `proto_ops`, FFI symbol names, Makefile targets, and user-space tests are
unchanged. The refactor should therefore preserve all existing behavior.
