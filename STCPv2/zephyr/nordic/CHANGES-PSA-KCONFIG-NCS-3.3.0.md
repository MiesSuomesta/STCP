# PSA Kconfig fix for NCS 3.3.0

The NCS 3.3.0 generated symbol `PSA_CRYPTO_CLIENT` has no prompt and cannot be assigned directly from `ethernet.conf`.

Changes:

- Removed `CONFIG_PSA_CRYPTO_CLIENT=y` from `stcp-mqtt/ethernet.conf`.
- Changed `STCP_RUST_CORE` to select the user-facing `MBEDTLS_PSA_CRYPTO_C` feature.
- The hidden `PSA_CRYPTO_CLIENT` value is now derived automatically by the NCS/TF-M Kconfig chain.

Build with a pristine build directory:

```bash
rm -rf stcp-mqtt/build-ethernet
cd stcp-mqtt
bash scripts/build-ethernet.sh
```
