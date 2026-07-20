#!/usr/bin/env bash
set -euo pipefail
APP_DIR="$(cd "$(dirname "$0")/.." && pwd)"
west flash -d "$APP_DIR/build-nrf9151" "$@"
