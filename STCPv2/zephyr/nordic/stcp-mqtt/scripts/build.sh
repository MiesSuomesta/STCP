#!/usr/bin/env bash
set -euo pipefail

NORDIC_DIR="/home/pomo/git/STCP/STCPv2/zephyr/nordic"
NCS_DIR="$NORDIC_DIR/ncs-3.3.0"
APP_DIR="$NORDIC_DIR/stcp-mqtt"
STCP_MODULE_DIR="$NORDIC_DIR/stcp-module"

VENV_DIR="$NCS_DIR/.venv"
VENV_PYTHON="$VENV_DIR/bin/python"
SDK_DIR="$HOME/zephyr-sdk-0.16.8"
BUILD_DIR="$APP_DIR/build-nrf9151"
BOARD="nrf9151dk/nrf9151/ns"

if [[ ! -x "$VENV_PYTHON" ]]; then
    echo "[FAIL] Python virtual environment missing: $VENV_PYTHON" >&2
    echo "Create it in $NCS_DIR before building." >&2
    exit 1
fi

if [[ ! -d "$SDK_DIR" ]]; then
    echo "[FAIL] Zephyr SDK missing: $SDK_DIR" >&2
    exit 1
fi

if [[ ! -d "$STCP_MODULE_DIR" ]]; then
    echo "[FAIL] STCP module missing: $STCP_MODULE_DIR" >&2
    exit 1
fi

cd "$NCS_DIR"
rm -rf "$BUILD_DIR"

# Nordic Toolchain Manager may export PYTHONHOME/PYTHONPATH pointing to its
# bundled Python. Mixing those with this venv causes: SRE module mismatch.
unset PYTHONHOME
unset PYTHONPATH

printf '[INFO] Python: '
"$VENV_PYTHON" --version
printf '[INFO] West:   '
"$VENV_PYTHON" -m west --version

"$VENV_PYTHON" -m west build \
    -p always \
    -b "$BOARD" \
    "$APP_DIR" \
    -d "$BUILD_DIR" \
    -- \
    -DZEPHYR_EXTRA_MODULES="$STCP_MODULE_DIR" \
    -DEXTRA_CONF_FILE="$APP_DIR/nrf9151.conf" \
    -DZEPHYR_SDK_INSTALL_DIR="$SDK_DIR"
