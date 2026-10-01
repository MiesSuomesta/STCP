STCP kernel crypto uses the kernel AES-GCM library functions aesgcm_expandkey(), 
aesgcm_encrypt() and aesgcm_decrypt(). Raspberry Pi 6.18 defines CRYPTO_LIB_AESGCM 
as a promptless Kconfig symbol and ARM64 does not select it. Without it, STCP builds
until MODPOST, which then fails with undefined aesgcm_* symbols. The RPi preparation
patch exposes the option and the preparation script forces it built-in (=y) together
with its selected dependencies.

