#!/usr/bin/env bash
set -euo pipefail
PROJECT_DIR="/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-module"
VENV_DIR="/home/pomo/git/STCP/STCPv2/zephyr/nordic/.venv"
cd "$PROJECT_DIR"
source "$VENV_DIR/bin/activate"
west flash -d build-nrf9151-mqtt "$@"
