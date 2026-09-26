#!/usr/bin/env bash
set -Eeuo pipefail
source /srv/stcp-project/settings.sh
fail() { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

APP_NAME="${1:-application}"
case "$APP_NAME" in
    application|app-coap|app-mqtt|p2p-application) ;;
    *) fail "Unknown application: $APP_NAME" ;;
esac


VERSION="$(readlink -f "$STCP_ROOT")"
NORDIC_ROOT="$VERSION/zephyr/nordic"
APP_DIR="$NORDIC_ROOT/$APP_NAME"
MODULE_DIR="$NORDIC_ROOT/module-v2"
BUILD_DIR="$APP_DIR/build-ethernet"
BOARD="nrf9151dk/nrf9151/ns"
SHIELD="seeed_w5500"
OVERLAY="$APP_DIR/boards/nrf9151dk_nrf9151_ns_w5500.overlay"
COMMON_CONF="$NORDIC_ROOT/common.conf"
TRANSPORT_CONF="$APP_DIR/ethernet.conf"

west build --sysbuild -p always -d "$BUILD_DIR" -b "$BOARD" \
    --shield "$SHIELD" "$APP_DIR" -- \
    "-DZEPHYR_EXTRA_MODULES=$MODULE_DIR" \
    "-DEXTRA_CONF_FILE=$COMMON_CONF;$TRANSPORT_CONF" \
    "-DDTC_OVERLAY_FILE=$OVERLAY" \
    "-DZEPHYR_SDK_INSTALL_DIR=$ZEPHYR_SDK_INSTALL_DIR"

for f in "$APP_DIR/CMakeLists.txt" "$APP_DIR/prj.conf" "$COMMON_CONF" "$TRANSPORT_CONF" \
         "$MODULE_DIR/zephyr/module.yml" "$OVERLAY"; do
    [[ -e "$f" ]] || fail "Missing: $f"
done
