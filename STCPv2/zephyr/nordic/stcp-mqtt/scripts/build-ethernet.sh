#!/usr/bin/env bash
set -euo pipefail

APP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
NORDIC_DIR="$(cd "$APP_DIR/.." && pwd)"
NCS_DIR="$NORDIC_DIR/ncs-3.3.0"
STCP_MODULE_DIR="$NORDIC_DIR/stcp-module"
PYTHON="$NCS_DIR/.venv/bin/python"
SDK_DIR="${ZEPHYR_SDK_INSTALL_DIR:-$HOME/ncs/toolchains/911f4c5c26/opt/zephyr-sdk}"
BUILD_DIR="$APP_DIR/build-ethernet"
BOARD="nrf9151dk/nrf9151/ns"

unset PYTHONHOME PYTHONPATH

echo "[INFO] Using existing W5500 driver unchanged"

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
