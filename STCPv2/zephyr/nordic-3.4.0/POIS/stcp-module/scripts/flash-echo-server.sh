#!/usr/bin/env bash
set -euo pipefail

PROJECT_DIR="/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-module"
VENV_DIR="/home/pomo/git/STCP/STCPv2/zephyr/nordic/.venv"
BUILD_DIR="build-stcp-echo-server"

cd "$PROJECT_DIR"
# shellcheck disable=SC1091
source "$VENV_DIR/bin/activate"

west flash -d "$BUILD_DIR" "$@"
