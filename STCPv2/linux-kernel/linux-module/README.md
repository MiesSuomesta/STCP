# STCP kernel module: X25519 + ChaCha20-Poly1305 phase

This version extends the working symmetric handshake implementation with real
kernel cryptography while preserving the existing BSD socket wrappers and
loopback carrier.

## Implemented

- Linux BSD socket interface remains unchanged.
- Symmetric `PublicKey` / `HandshakeDone` exchange.
- Curve25519 key pair generation through the Linux kernel crypto helpers.
- X25519 shared-secret derivation.
- ChaCha20-Poly1305 authenticated encryption for every data frame.
- STCP header used as AEAD associated data.
- 64-bit directional nonce spaces:
  - client/A direction starts at `0`
  - server/B direction starts at `1 << 63`
- Strict receive-side nonce ordering.
- 16-byte authentication tags.
- Authentication, replay/order, and malformed-frame errors move the socket to
  `Error`.
- Key material is zeroed when the Rust crypto context is dropped.
- Basic and 200 KiB framed tests remain available through `make test`.

The public-key wire payload remains 64 bytes for compatibility with the earlier
STCP format. Curve25519 uses the first 32 bytes; the remaining bytes are zero.

## Kernel requirements

The target kernel must provide/export:

- Curve25519 helpers from `<crypto/curve25519.h>`
- ChaCha20-Poly1305 helpers from `<crypto/chacha20poly1305.h>`

Typical relevant kernel configuration options include Curve25519 and
ChaCha20-Poly1305 crypto support.

## Build and test

```bash
make LLVM=1 V=1 test
```

To inspect required symbols before loading:

```bash
nm -u stcp.ko | grep -E 'curve25519|chacha20poly1305'
grep -E 'curve25519|chacha20poly1305' /proc/kallsyms
```

## Wire format for encrypted data frames

```text
16-byte authenticated STCP header
8-byte big-endian nonce
ciphertext
16-byte Poly1305 tag
```

`PublicKey`, `HandshakeDone`, and `Close` remain control frames in this phase.
The next hardening phase should authenticate the handshake transcript and derive
separate directional traffic keys with a KDF.
