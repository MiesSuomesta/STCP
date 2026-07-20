#!/usr/bin/env bash
set -euo pipefail
PROJECT_DIR="/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-module"
VENV_DIR="/home/pomo/git/STCP/STCPv2/zephyr/nordic/.venv"
SDK_DIR="${HOME}/zephyr-sdk-1.0.1"
cd "$PROJECT_DIR"
source "$VENV_DIR/bin/activate"
rm -rf build-nrf9151-mqtt
west build --no-sysbuild -p always \
  -d build-nrf9151-mqtt \
  -b nrf9151dk/nrf9151 \
  samples/stcp_mqtt_proxy -- \
  -DZEPHYR_EXTRA_MODULES="$PROJECT_DIR" \
  -DZEPHYR_SDK_INSTALL_DIR="$SDK_DIR"
