#!/usr/bin/env bash
set -Eeuo pipefail
APP_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
REPO_ROOT="$(cd "$APP_ROOT/../../.." && pwd -P)"
MODULE="$REPO_ROOT/zephyr/nordic/module-v2"
BUILD="${STCP_V2_BUILD_DIR:-$APP_ROOT/build-v2-clean}"
BOARD="${STCP_V2_BOARD:-nrf9151dk/nrf9151/ns}"

west build -p always -d "$BUILD" -b "$BOARD" "$APP_ROOT" -- \
    -DZEPHYR_EXTRA_MODULES="$MODULE" \
    -DEXTRA_CONF_FILE="${STCP_V2_CONF:-$APP_ROOT/ethernet.conf;$APP_ROOT/stcp-v2-clean.conf}"
