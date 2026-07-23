#!/usr/bin/env bash
set -euo pipefail

NORDIC_DIR="/home/pomo/git/STCP/STCPv2/zephyr/nordic"
NCS_DIR="$NORDIC_DIR/ncs-3.3.0"
APP_DIR="$NORDIC_DIR/stcp-mqtt"
STCP_MODULE_DIR="$NORDIC_DIR/stcp-module"

VENV_DIR="$NCS_DIR/.venv"
SDK_DIR="$HOME/zephyr-sdk-0.16.8"
BUILD_DIR="$APP_DIR/build-nrf9151"
BOARD="nrf9151dk/nrf9151/ns"

source "$VENV_DIR/bin/activate"

cd "$NCS_DIR"

rm -rf "$BUILD_DIR"

python -m west build \
    -p always \
    -b "$BOARD" \
    "$APP_DIR" \
    -d "$BUILD_DIR" \
    -- \
    -DZEPHYR_EXTRA_MODULES="$STCP_MODULE_DIR" \
    -DEXTRA_CONF_FILE="$APP_DIR/nrf9151.conf" \
    -DZEPHYR_SDK_INSTALL_DIR="$SDK_DIR"

