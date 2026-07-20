#!/usr/bin/env bash
set -euo pipefail
MODULE_DIR="$(cd -- "$(dirname -- "$0")/.." && pwd)"
APP_DIR="$MODULE_DIR/samples/stcp_offload_stub"
west build -p always -b native_sim/native "$APP_DIR" -- -DZEPHYR_EXTRA_MODULES="$MODULE_DIR"
