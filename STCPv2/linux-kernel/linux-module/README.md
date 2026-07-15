# STCP directional session keys

This stage extends the working X25519 + ChaCha20-Poly1305 transport with:

- HKDF-SHA256 key derivation implemented in `no_std` Rust
- separate client-to-server and server-to-client keys
- both directions start nonce counters from zero because keys are distinct
- exact monotonically increasing receive nonce checks
- TX nonce advances only after successful encryption
- RX nonce advances only after successful authentication
- replay, reordering and skipped nonces are rejected
- module-load crypto selftest
- tampered authentication tag selftest
- wrong-direction key selftest
- key material zeroing on drop

The BSD socket wrappers and loopback carrier are unchanged.

## Build and test

```bash
make LLVM=1 V=1 test
```

The test target loads:

```bash
modprobe libcurve25519
modprobe libchacha20poly1305
```

before inserting `stcp.ko`.

Expected kernel log includes:

```text
stcp: directional crypto selftest passed
```

## Wire protection

Each data frame authenticates its complete 16-byte STCP header as AAD. The wire
payload is:

```text
8-byte big-endian nonce || ciphertext || 16-byte Poly1305 tag
```

A receiver accepts only the exact expected nonce. The expected nonce is updated
only after successful Poly1305 authentication.
