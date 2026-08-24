STCPv2 Zephyr PSA X25519

Replaces the Zephyr software RFC7748 X25519 path with PSA Crypto:
  stcp_kernel_x25519_keypair()
    -> psa_generate_key()
    -> psa_export_key()
    -> psa_export_public_key()

  stcp_kernel_x25519_shared()
    -> psa_import_key()
    -> psa_raw_key_agreement(PSA_ALG_ECDH)

The bundled stcp_x25519_soft.c is removed from the normal module-v2 build.
This specifically removes the function that the Zephyr fault decoded to:
  stcp_x25519_soft
  stcp_kernel_x25519_shared

The previous PSA SHA256/HMAC session-KDF changes are preserved in
stcp_v2_platform.c and Kconfig.

Install this ZIP at the STCPv2 repository root:
  /home/pomo/git/github/STCP/STCPv2

Then do a pristine Zephyr build:
  cd ~/zephyr-stcp/stcp/application
  rm -rf build-v2-clean
  bash scripts/build-v2-clean.sh

Before flashing, useful checks:
  grep -E 'PSA_WANT_ALG_ECDH|PSA_WANT_ECC_MONTGOMERY_255'     build-v2-clean/application/zephyr/.config

  strings build-v2-clean/application/zephyr/zephyr.elf |     grep -E 'X25519 keypair start: backend=PSA|X25519 backend selected: PSA'

  nm build-v2-clean/application/zephyr/zephyr.elf | grep stcp_x25519_soft
The last command should return nothing.
