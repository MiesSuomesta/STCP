#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-module"
VENV_DIR="/home/pomo/git/STCP/STCPv2/zephyr/nordic/.venv"
SDK_DIR="${HOME}/zephyr-sdk-1.0.1"
BUILD_DIR="build-stcp-echo-server"
BOARD="nrf9151dk/nrf9151"
SAMPLE="samples/stcp_echo_server"

cd "$PROJECT_DIR"
# shellcheck disable=SC1091
source "$VENV_DIR/bin/activate"

rm -rf "$BUILD_DIR"

west build \
    --no-sysbuild \
    -p always \
    -d "$BUILD_DIR" \
    -b "$BOARD" \
    "$SAMPLE" \
    -- \
    -DZEPHYR_EXTRA_MODULES="$PROJECT_DIR" \
    -DZEPHYR_SDK_INSTALL_DIR="$SDK_DIR"

echo "Built: $PROJECT_DIR/$BUILD_DIR/zephyr/zephyr.hex"
