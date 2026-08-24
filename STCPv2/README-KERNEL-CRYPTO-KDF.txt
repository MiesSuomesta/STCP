STCPv2 platform crypto KDF migration
======================================

Purpose
-------
Remove the software SHA256/HMAC KDF from the Rust shared core and delegate
session-key derivation to the platform crypto backend.

The wire/key derivation is intentionally unchanged:

  salt = SHA256("STCPv2-HKDF-SHA256" || client_pub || server_pub)
  prk  = HMAC-SHA256(salt, shared_secret)
  c2s  = HMAC-SHA256(prk, "STCPv2 client to server key" || 0x01)
  s2c  = HMAC-SHA256(prk, "STCPv2 server to client key" || 0x01)

Linux/Raspberry Pi
------------------
Uses Linux kernel crypto_shash:
  sha256
  hmac(sha256)

The shash descriptor is heap allocated, so algorithm scratch space is not
placed on the caller's stack.

Zephyr/nRF9151
--------------
Uses PSA Crypto:
  PSA_ALG_SHA_256
  PSA_ALG_HMAC(PSA_ALG_SHA_256)

Concatenation scratch is allocated with k_malloc and wiped/freed after use.
This removes the Rust software SHA256 compress() w[64] array and HMAC scratch
from the stcp-v2-rx thread stack.

Files
-----
kernel/module/rust/src/crypto.rs
kernel/module/rust/src/lib.rs
kernel/module/src/stcp_crypto.c
kernel/module/include/stcp_crypto.h
kernel/module/Kconfig
zephyr/nordic/module-v2/src/stcp_v2_platform.c
zephyr/nordic/module-v2/Kconfig

Install
-------
Extract at:
  /home/pomo/git/github/STCP/STCPv2

Then rebuild Linux/RPi and Zephyr.  For Zephyr use a pristine build because
old ZIP mtimes previously caused Ninja to retain stale objects.

Note
----
kdf.rs is deliberately left in the repository but lib.rs no longer declares
mod kdf, so it is not compiled into the shared core. This makes rollback easy.
