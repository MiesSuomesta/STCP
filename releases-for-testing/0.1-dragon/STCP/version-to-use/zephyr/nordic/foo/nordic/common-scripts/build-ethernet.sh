#!/usr/bin/env bash
set -Eeuo pipefail
source /srv/stcp-project/settings.sh
die() { printf '[FAIL] %s\n' "$*" >&2; exit 1; }
(( $# == 1 )) || die "Usage: $0 {application|app-coap|app-mqtt|p2p-application}"
APP_NAME="$1"
case "$APP_NAME" in application|app-coap|app-mqtt|p2p-application) ;; *) die "Unknown application: $APP_NAME" ;; esac

VERSION="$(readlink -f "$STCP_ROOT")"
NORDIC_ROOT="$VERSION/zephyr/nordic"
WEST_ROOT="${ZEPHYR_WORKSPACE:-/srv/stcp-project/zephyr-stcp}"
APP_DIR="$NORDIC_ROOT/$APP_NAME"
MODULE_DIR="$NORDIC_ROOT/module-v2"
COMMON_CONF="$NORDIC_ROOT/common.conf"
TRANSPORT_CONF="$NORDIC_ROOT/ethernet.conf"
BUILD_DIR="$APP_DIR/build-ethernet"
PYTHON="${WEST_PYTHON:-$WEST_ROOT/.venv/bin/python}"
SDK_DIR="${ZEPHYR_SDK_INSTALL_DIR:-$WEST_ROOT/../zephyr-sdk-0.16.8}"
BOARD="nrf9151dk/nrf9151/ns"
SHIELD="seeed_w5500"
DTC_OVERLAY_FILE="$APP_DIR/boards/nrf9151dk_nrf9151_ns_w5500.overlay"

for f in "$APP_DIR/CMakeLists.txt" "$APP_DIR/prj.conf" "$COMMON_CONF" "$TRANSPORT_CONF" \
         "$MODULE_DIR/zephyr/module.yml" "$DTC_OVERLAY_FILE"; do
    [[ -e "$f" ]] || die "Missing: $f"
done

unset PYTHONHOME PYTHONPATH
cd "$WEST_ROOT"
exec "$PYTHON" -m west build --sysbuild -p always -d "$BUILD_DIR" -b "$BOARD" \
    --shield "$SHIELD" "$APP_DIR" -- \
    "-DZEPHYR_EXTRA_MODULES=$MODULE_DIR" \
    "-DEXTRA_CONF_FILE=$COMMON_CONF;$TRANSPORT_CONF" \
    "-DDTC_OVERLAY_FILE=$DTC_OVERLAY_FILE" \
    "-DZEPHYR_SDK_INSTALL_DIR=$SDK_DIR"
