# Rust STCP X25519 software backend

NCS 3.3.0 / TF-M PSA returns `PSA_ERROR_NOT_SUPPORTED` for both Montgomery
X25519 key generation and raw private-key import. Therefore PSA cannot be used
for X25519 on this target.

This revision:

- adds a bundled constant-time RFC 7748 X25519 implementation,
- uses PSA only for 32-byte cryptographic random generation,
- computes the X25519 public key and shared secret locally,
- rejects an all-zero public/shared result,
- retains PSA ChaCha20-Poly1305,
- logs the selected backend and public wire values,
- never logs private scalars, shared secrets, or session keys.

The implementation was checked against the RFC 7748 Alice public-key test
vector:

`8520f0098930a754748b7ddcb43ef75a0dbf3a0d26381af4eba4a98eaa9b4e6a`
