#!/usr/bin/env bash
set -euo pipefail

APP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
NORDIC_DIR="$(cd "$APP_DIR/.." && pwd)"
REPO_ROOT="$(cd "$NORDIC_DIR/../.." && pwd)"
OUT="${1:-$PWD/stcp-zephyr-source.zip}"

cd "$REPO_ROOT"
zip -r "$OUT" zephyr/nordic/application zephyr/nordic/module \
    -x '*/build-*/*' '*/__pycache__/*' '*/target/*'
