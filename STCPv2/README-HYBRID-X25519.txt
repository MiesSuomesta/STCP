STCPv2 Zephyr hybrid X25519 backend

Why hybrid?
-----------
On the tested NCS 3.3.0 / nRF9151 build:
  psa_generate_key(Montgomery255/ECDH) -> PSA_ERROR_NOT_SUPPORTED (-134)

The existing software RFC7748 keypair path already works during AF_STCP socket
creation. The actual stack overflow occurs later, in stcp-v2-rx, while computing
the shared secret inside stcp_x25519_soft().

This overlay therefore uses:

  keypair:
    PSA random
      -> clamp
      -> bundled RFC7748 public-key calculation

  shared secret:
    psa_import_key(Montgomery255 private key)
      -> psa_raw_key_agreement(PSA_ALG_ECDH)

This removes the expensive software scalar multiplication from the RX thread
without depending on unsupported PSA X25519 key generation.

Required PSA feature selections:
  PSA_WANT_ALG_ECDH
  PSA_WANT_ECC_MONTGOMERY_255
  PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_IMPORT
  PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_DERIVE

Install at STCPv2 repository root, then pristine-build Zephyr.
