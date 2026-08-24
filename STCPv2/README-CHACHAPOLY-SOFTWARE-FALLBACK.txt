STCPv2 Zephyr ChaCha20-Poly1305 fallback
========================================

Based on the supplied current zephyr/nordic/module-v2/src/stcp_v2_platform.c.

Behavior:
- PSA ChaCha20-Poly1305 remains the preferred backend.
- ONLY PSA_ERROR_NOT_SUPPORTED triggers the software fallback.
- Other PSA failures remain failures; authentication errors are not hidden.
- Encrypt and decrypt both support the fallback.
- decrypt_in_place automatically benefits because it calls decrypt().
- Software implementation follows RFC 8439 ChaCha20-Poly1305.
- Software path is allocation-free, important with the reduced 32 KiB heap.
- Constant-time 16-byte authentication tag comparison is used.
- No secret/session-key material is logged.

Local verification performed before packaging:
- software ciphertext + Poly1305 tag matched Python cryptography's
  ChaCha20Poly1305 for a non-trivial test vector (AAD 37 B, plaintext 131 B).
- software decrypt round-trip passed.

Expected Zephyr log when PSA lacks the AEAD:
  ChaCha20-Poly1305 encrypt: PSA unsupported, using software RFC8439 fallback
