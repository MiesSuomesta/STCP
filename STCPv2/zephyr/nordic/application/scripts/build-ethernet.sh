#!/usr/bin/env bash
set -euo pipefail

APP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# NCS 3.3.0: PSA_CRYPTO_CLIENT is a hidden generated symbol and must not
# be assigned from application configuration files.
if grep -Rqs '^CONFIG_PSA_CRYPTO_CLIENT=' "$APP_DIR"/*.conf 2>/dev/null; then
    echo "[FAIL] CONFIG_PSA_CRYPTO_CLIENT is a hidden NCS symbol; remove the direct assignment." >&2
    exit 1
fi

NORDIC_DIR="$(cd "$APP_DIR/.." && pwd)"
NCS_DIR="${NCS_DIR:-$NORDIC_DIR/ncs-3.3.0}"
STCP_MODULE_DIR="${STCP_MODULE_DIR:-$NORDIC_DIR/module}"
PYTHON="$NCS_DIR/.venv/bin/python"
SDK_DIR="${ZEPHYR_SDK_INSTALL_DIR:-$HOME/ncs/toolchains/911f4c5c26/opt/zephyr-sdk}"
BUILD_DIR="$APP_DIR/build-ethernet"
BOARD="nrf9151dk/nrf9151/ns"

unset PYTHONHOME PYTHONPATH

if grep -q '^CONFIG_STCP_RUST_CORE=y' "$APP_DIR/ethernet.conf"; then
    "$STCP_MODULE_DIR/scripts/check-rust-integration.sh"
fi

"$PYTHON" "$APP_DIR/scripts/patch-w5500-driver.py" \
    "$NCS_DIR/zephyr/drivers/ethernet/eth_w5500.c"

rm -rf "$BUILD_DIR"

exec "$PYTHON" -m west build \
    -p always \
    -b "$BOARD" \
    --sysbuild \
    --shield seeed_w5500 \
    -d "$BUILD_DIR" \
    "$APP_DIR" \
    -- \
    -DZEPHYR_EXTRA_MODULES="$STCP_MODULE_DIR" \
    -DCONF_FILE="$APP_DIR/ethernet.conf" \
    -DZEPHYR_SDK_INSTALL_DIR="$SDK_DIR"
