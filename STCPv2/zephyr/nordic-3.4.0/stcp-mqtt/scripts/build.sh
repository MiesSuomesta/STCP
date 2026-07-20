#!/usr/bin/env bash
set -euo pipefail
APP_DIR="$(cd "$(dirname "$0")/.." && pwd)"
MODULE_DIR="$(cd "$APP_DIR/../stcp-module" && pwd)"
WEST_TOP="$(cd "$APP_DIR/.." && pwd)"
source "$WEST_TOP/.venv/bin/activate"
rm -rf "$APP_DIR/build-nrf9151"
west build -p always -d "$APP_DIR/build-nrf9151" -b nrf9151dk/nrf9151/ns "$APP_DIR" -- \
  -DZEPHYR_EXTRA_MODULES="$MODULE_DIR" \
  -DEXTRA_CONF_FILE="$APP_DIR/nrf9151.conf" \
  -DZEPHYR_SDK_INSTALL_DIR="$HOME/zephyr-sdk-1.0.1"
