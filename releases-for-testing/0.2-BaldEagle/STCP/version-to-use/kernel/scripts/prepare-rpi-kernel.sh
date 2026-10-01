#!/bin/bash
set -euo pipefail

GIT_ROOT="$(git rev-parse --show-toplevel 2>/dev/null)" || {
    echo "[FAIL] Not inside STCP git repository"
    exit 1
}

STCP_ROOT="$GIT_ROOT/version-to-use"

RPI_KERNEL="$STCP_ROOT/kernel/raspberry"
CONFIG="$RPI_KERNEL/.config"

echo "[INFO] Git root:   $GIT_ROOT"
echo "[INFO] RPi kernel: $RPI_KERNEL"

[[ -d "$RPI_KERNEL" ]] || {
    echo "[FAIL] RPi kernel tree not found: $RPI_KERNEL"
    exit 1
}

[[ -f "$CONFIG" ]] || {
    echo "[FAIL] RPi kernel config not found: $CONFIG"
    exit 1
}

# Patch Kconfig only if the AESGCM option is still promptless.
echo "[INFO] Ensuring AES-GCM Kconfig option is user-selectable..."

python3 - "$RPI_KERNEL/lib/crypto/Kconfig" <<'PY'
from pathlib import Path
import sys

p = Path(sys.argv[1])
s = p.read_text()

block = """config CRYPTO_LIB_AESGCM
\ttristate
"""

patched = """config CRYPTO_LIB_AESGCM
\ttristate
\tprompt "AES-GCM library"
"""

if patched in s:
    print("[OK] AES-GCM Kconfig prompt already present")
elif block in s:
    p.write_text(s.replace(block, patched, 1))
    print("[OK] AES-GCM Kconfig prompt added")
else:
    raise SystemExit(
        "[FAIL] Unexpected CRYPTO_LIB_AESGCM Kconfig layout"
    )
PY

echo "[INFO] Pre-oldconfig: Verifying Raspberry Pi ARM64 config..."

if ! grep -q '^CONFIG_ARM64=y$' "$CONFIG"; then
    echo "[FAIL] Raspberry Pi config is not ARM64: $CONFIG" >&2
    exit 1
fi

if grep -q '^CONFIG_X86_64=y$' "$CONFIG"; then
    echo "[FAIL] Raspberry Pi config contains CONFIG_X86_64=y: $CONFIG" >&2
    exit 1
fi

echo "[OK] Raspberry Pi config architecture: ARM64"

echo "[INFO] Enabling RPi AES-GCM crypto library..."

"$RPI_KERNEL/scripts/config" \
    --file "$CONFIG" \
    --enable CRYPTO_LIB_AESGCM

make -C "$RPI_KERNEL" \
    ARCH=arm64 \
    CROSS_COMPILE=aarch64-linux-gnu- \
    olddefconfig

echo "[INFO] Post-oldconfig: Verifying Raspberry Pi ARM64 config..."

echo "[INFO] Verifying Raspberry Pi ARM64 config..."

if ! grep -q '^CONFIG_ARM64=y$' "$CONFIG"; then
    echo "[FAIL] Raspberry Pi config is not ARM64: $CONFIG" >&2
    exit 1
fi

if grep -q '^CONFIG_X86_64=y$' "$CONFIG"; then
    echo "[FAIL] Raspberry Pi config contains CONFIG_X86_64=y: $CONFIG" >&2
    exit 1
fi

echo "[OK] Raspberry Pi config architecture: ARM64"

grep -q '^CONFIG_CRYPTO_LIB_AES=y$' "$CONFIG" || {
    echo "[FAIL] CONFIG_CRYPTO_LIB_AES != y"
    exit 1
}

grep -q '^CONFIG_CRYPTO_LIB_AESGCM=y$' "$CONFIG" || {
    echo "[FAIL] CONFIG_CRYPTO_LIB_AESGCM != y"
    exit 1
}

grep -q '^CONFIG_CRYPTO_LIB_GF128MUL=y$' "$CONFIG" || {
    echo "[FAIL] CONFIG_CRYPTO_LIB_GF128MUL != y"
    exit 1
}

echo
echo "[OK] RPi STCP crypto prerequisites:"
grep -E '^CONFIG_CRYPTO_LIB_(AES|AESGCM|GF128MUL)=' "$CONFIG"
