#!/usr/bin/env bash
set -Eeuo pipefail
source /srv/stcp-project/settings.sh
log() { printf '[INFO] %s\n' "$*"; }
fail() { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

VERSION_TO_USE="$(readlink -f "$STCP_ROOT")"
NORDIC_ROOT="$VERSION_TO_USE/zephyr/nordic"
APP_ROOT="$NORDIC_ROOT/application"
MODULE_V2="$NORDIC_ROOT/module-v2"
CANONICAL_CORE="$VERSION_TO_USE/kernel/module/rust"
COMMON_CONF="$NORDIC_ROOT/common.conf"
TRANSPORT_CONF="$NORDIC_ROOT/lte.conf"
BUILD_DIR="$APP_ROOT/build-lte"
BOARD="nrf9151dk/nrf9151/ns"

for f in "$APP_ROOT/CMakeLists.txt" "$APP_ROOT/prj.conf" "$COMMON_CONF" "$TRANSPORT_CONF" \
         "$MODULE_V2/zephyr/module.yml" "$CANONICAL_CORE/Cargo.toml"; do
    [[ -e "$f" ]] || fail "Missing: $f"
done

export STCP_SHARED_RUST_CORE_DIR="$CANONICAL_CORE"
export STCP_CANONICAL_CORE="$CANONICAL_CORE"
export ZEPHYR_SDK_INSTALL_DIR

log "Application : $APP_ROOT"
log "Common conf : $COMMON_CONF"
log "LTE conf    : $TRANSPORT_CONF"
log "Build dir   : $BUILD_DIR"

west build -p always -d "$BUILD_DIR" -b "$BOARD" "$APP_ROOT" -- \
    "-DZEPHYR_EXTRA_MODULES=$MODULE_V2" \
    "-DEXTRA_CONF_FILE=$COMMON_CONF;$TRANSPORT_CONF" \
    "-DSTCP_SHARED_RUST_CORE_DIR=$CANONICAL_CORE" \
    "-DSTCP_CANONICAL_CORE=$CANONICAL_CORE" \
    "-DZEPHYR_SDK_INSTALL_DIR=$ZEPHYR_SDK_INSTALL_DIR"

CONFIG_FILE="$BUILD_DIR/application/zephyr/.config"
[[ -f "$CONFIG_FILE" ]] || CONFIG_FILE="$BUILD_DIR/zephyr/.config"
[[ -f "$CONFIG_FILE" ]] || fail "Resulting .config not found"

grep -q '^CONFIG_NRF_MODEM_LIB=y' "$CONFIG_FILE" || fail "CONFIG_NRF_MODEM_LIB=y missing"
grep -q '^CONFIG_LTE_LINK_CONTROL=y' "$CONFIG_FILE" || fail "CONFIG_LTE_LINK_CONTROL=y missing"
grep -q '^CONFIG_NET_SOCKETS_OFFLOAD=y' "$CONFIG_FILE" || fail "CONFIG_NET_SOCKETS_OFFLOAD=y missing"
grep -q '^CONFIG_NET_NATIVE=y' "$CONFIG_FILE" && fail "CONFIG_NET_NATIVE=y leaked into LTE build"
log "DONE"
