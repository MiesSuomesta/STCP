#!/usr/bin/env bash
set -euo pipefail

APP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
NORDIC_DIR="$(cd "$APP_DIR/.." && pwd)"
NCS_DIR="${NCS_DIR:-$NORDIC_DIR/ncs-3.3.0}"
STCP_MODULE_DIR="${STCP_MODULE_DIR:-$NORDIC_DIR/module}"

VENV_DIR="$NCS_DIR/.venv"
VENV_PYTHON="$VENV_DIR/bin/python"
SDK_DIR="${ZEPHYR_SDK_INSTALL_DIR:-$HOME/zephyr-sdk-0.16.8}"
BUILD_DIR="$APP_DIR/build-nrf9151"
BOARD="nrf9151dk/nrf9151/ns"

[[ -x "$VENV_PYTHON" ]] || { echo "[FAIL] Python virtual environment missing: $VENV_PYTHON" >&2; exit 1; }
[[ -d "$SDK_DIR" ]] || { echo "[FAIL] Zephyr SDK missing: $SDK_DIR" >&2; exit 1; }
[[ -d "$STCP_MODULE_DIR" ]] || { echo "[FAIL] STCP module missing: $STCP_MODULE_DIR" >&2; exit 1; }

rm -rf "$BUILD_DIR"
unset PYTHONHOME PYTHONPATH
printf '[INFO] Python: '; "$VENV_PYTHON" --version
printf '[INFO] West:   '; "$VENV_PYTHON" -m west --version

exec "$VENV_PYTHON" -m west build \
    -p always \
    -b "$BOARD" \
    "$APP_DIR" \
    -d "$BUILD_DIR" \
    -- \
    -DZEPHYR_EXTRA_MODULES="$STCP_MODULE_DIR" \
    -DEXTRA_CONF_FILE="$APP_DIR/nrf9151.conf" \
    -DZEPHYR_SDK_INSTALL_DIR="$SDK_DIR"
