STCPv6 Zephyr Nordic crypto fix (source overlay)

From your STCP/STCPv6/zephyr/nordic directory:
  unzip -o stcpv6-nordic-crypto-patched.zip

Changes:
 - Shares existing RFC8439 software fallback in a local header.
 - Both Rust and v2 platform crypto paths use the same fallback.
 - Rust PSA streaming uses fallback only on PSA_ERROR_NOT_SUPPORTED.
 - Clears decrypt output on errors before returning.
 - Validates AAD pointer and plaintext length.

Rebuild Zephyr Ethernet (pristine) and run crypto selftests and Linux-to-nRF benchmark.
NOT hardware-validated. Keep a git commit or backup before overwriting.
