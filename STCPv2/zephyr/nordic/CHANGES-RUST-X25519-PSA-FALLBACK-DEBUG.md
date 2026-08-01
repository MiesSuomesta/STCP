# Rust STCP X25519 / PSA fallback and diagnostics

This update fixes the `socket(AF_STCP) -> ENOTSUP` failure caused by X25519
key generation/export during Rust `CryptoContext::new()`.

## Changes

- Initializes PSA once at application startup and rechecks lazily from each
  crypto operation.
- Logs the exact PSA operation and status for:
  - `psa_generate_key`
  - `psa_export_key`
  - `psa_export_public_key`
  - `psa_import_key`
  - `psa_raw_key_agreement`
- Adds `CONFIG_STCP_RUST_TRACE_CRYPTO=y`.
- Adds `CONFIG_STCP_RUST_X25519_RAW_FALLBACK=y`.
- If generated private-key export is unsupported, the fallback:
  1. generates a 32-byte scalar with `psa_generate_random()`;
  2. clamps it according to RFC 7748;
  3. imports it as an X25519/Montgomery key pair;
  4. exports only the public key;
  5. keeps PSA ECDH for shared-secret calculation.
- Adds readable PSA status names and detailed ChaCha20-Poly1305 errors.
- Crypto initialization failure no longer aborts system boot; the socket path
  reports the exact error.

## Expected log

```text
STCP crypto init: status=0 (PSA_SUCCESS)
X25519 keypair start: backend=PSA generated/exportable key
X25519 psa_generate_key: ...
X25519 psa_export_key: status=-134 (PSA_ERROR_NOT_SUPPORTED)
X25519 backend fallback: raw scalar + PSA import/public export
X25519 fallback psa_generate_random: status=0 (PSA_SUCCESS)
X25519 fallback psa_import_key: status=0 (PSA_SUCCESS)
X25519 fallback psa_export_public_key: status=0 (PSA_SUCCESS) len=32
X25519 backend selected: raw-secret PSA fallback
```
