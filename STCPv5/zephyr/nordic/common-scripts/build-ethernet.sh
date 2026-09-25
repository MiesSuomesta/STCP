#!/usr/bin/env bash
set -Eeuo pipefail

die() { printf '[FAIL] %s\n' "$*" >&2; exit 1; }
(( $# == 1 )) || die "Usage: $0 {application|app-coap|app-mqtt|p2p-application}"
APP_NAME="$1"
case "$APP_NAME" in application|app-coap|app-mqtt|p2p-application) ;; *) die "Unknown application: $APP_NAME" ;; esac

GIT_TOP="$(git rev-parse --show-toplevel)" || die "Run inside the STCP Git repository"
if [[ -d "$GIT_TOP/stcp/common-scripts" ]]; then
    STCP_ZEPHYR_ROOT="$GIT_TOP/stcp"
    WEST_ROOT="$GIT_TOP"
elif [[ -e "$GIT_TOP/STCP/version-to-use" || -e "$GIT_TOP/version-to-use" ]]; then
    if [[ -e "$GIT_TOP/STCP/version-to-use" ]]; then
        VERSION="$(readlink -f "$GIT_TOP/STCP/version-to-use")"
        PROJECT_ROOT="$GIT_TOP"
    else
        VERSION="$(readlink -f "$GIT_TOP/STCP/version-to-use")"
        PROJECT_ROOT="$GIT_TOP/.."
    fi
    STCP_ZEPHYR_ROOT="$VERSION/zephyr/nordic"
    WEST_ROOT="${ZEPHYR_WORKSPACE:-$PROJECT_ROOT/zephyr-stcp}"
else
    die "Cannot find stcp/ or STCP/version-to-use under $GIT_TOP"
fi

APP_DIR="$STCP_ZEPHYR_ROOT/$APP_NAME"
MODULE_DIR="${STCP_V2_MODULE:-$STCP_ZEPHYR_ROOT/module-v2}"
BUILD_DIR="${STCP_V2_BUILD_DIR:-$APP_DIR/build-ethernet}"
CONF_FILE="${STCP_V2_CONF:-$APP_DIR/ethernet.conf}"
PYTHON="${WEST_PYTHON:-$WEST_ROOT/.venv/bin/python}"
SDK_DIR="${ZEPHYR_SDK_INSTALL_DIR:-$WEST_ROOT/../zephyr-sdk-0.16.8}"
BOARD="${STCP_V2_BOARD:-nrf9151dk/nrf9151/ns}"
SHIELD="${STCP_V2_SHIELD:-seeed_w5500}"

[[ -f "$APP_DIR/CMakeLists.txt" ]] || die "Missing application: $APP_DIR"
[[ -f "$CONF_FILE" ]] || die "Missing config: $CONF_FILE"
[[ -f "$MODULE_DIR/zephyr/module.yml" ]] || die "Missing STCP module: $MODULE_DIR"
[[ -x "$PYTHON" ]] || die "Missing west Python: $PYTHON"
[[ -d "$SDK_DIR" ]] || die "Missing Zephyr SDK: $SDK_DIR"

printf '[INFO] Building %s: %s\n' "$APP_NAME" "$BUILD_DIR"
unset PYTHONHOME PYTHONPATH
cd "$WEST_ROOT"
exec "$PYTHON" -m west build --sysbuild -p always -d "$BUILD_DIR" -b "$BOARD" \
    --shield "$SHIELD" "$APP_DIR" -- \
    "-DZEPHYR_EXTRA_MODULES=$MODULE_DIR" \
    "-DEXTRA_CONF_FILE=$CONF_FILE" \
    "-DZEPHYR_SDK_INSTALL_DIR=$SDK_DIR"
