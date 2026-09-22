STCP native P2P Phase 2.1 - linker/source fix
==============================================

This overlay fixes the Phase-2 linker failure where the Zephyr application
referenced stcp_p2p_noise_* but the canonical Rust staticlib still contained
only the Phase-1 exports.

IMPORTANT PATHS
---------------
The module/ subtree MUST be copied over the canonical Linux/shared core:

  /home/pomo/git/github/STCP/STCPv2/kernel/module/

NOT over the Zephyr module-v2 directory.

The application/ subtree goes over:

  /home/pomo/zephyr-stcp/stcp/application/

The corrected Rust lib.rs explicitly includes BOTH:

  mod kdf;
  mod p2p;

Phase 2's p2p/noise.rs uses crate::kdf, so without mod kdf the new source
cannot actually be the source that produced a successful Rust staticlib.

Also fixed Noise selftest: Noise_XX_25519_ChaChaPoly_SHA256 is exactly 32
bytes, so Noise initializes h/ck directly from the protocol name rather than
SHA256(protocol_name).

After overlay:

  rm -rf /home/pomo/git/github/STCP/STCPv2/kernel/module/rust/target/thumbv8m.main-none-eabi/release
  cd /home/pomo/zephyr-stcp/stcp/application
  ./scripts/build-v2-clean.sh

Before flashing, verify the archive really contains the Phase-2 symbols:

  arm-zephyr-eabi-nm \
    /home/pomo/git/github/STCP/STCPv2/kernel/module/rust/target/thumbv8m.main-none-eabi/release/libstcp_kernel_core.a \
    | grep 'stcp_p2p_noise_'

Expected at least:
  stcp_p2p_noise_core_ready
  stcp_p2p_noise_selftest
  stcp_p2p_noise_dialer_new
  stcp_p2p_noise_dialer_free
  stcp_p2p_noise_dialer_message1
  stcp_p2p_noise_dialer_message2
  stcp_p2p_noise_dialer_complete
