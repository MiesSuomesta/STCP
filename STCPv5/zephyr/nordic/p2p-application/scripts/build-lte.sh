#!/usr/bin/env bash
set -Eeuo pipefail

source /srv/stcp-project/settings.sh

log()  { printf '[INFO] %s\n' "$*"; }
fail() { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

VERSION_TO_USE="$(readlink -f "$STCP_ROOT")"
[[ -d "$VERSION_TO_USE" ]] || fail "Missing STCP version: $STCP_ROOT"

NORDIC_ROOT="${NORDIC_ROOT:-$VERSION_TO_USE/zephyr/nordic}"
APP_ROOT="${STCP_V2_APP_ROOT:-$NORDIC_ROOT/application}"
MODULE_V2="${STCP_V2_MODULE:-$NORDIC_ROOT/module-v2}"
CANONICAL_CORE="${STCP_SHARED_RUST_CORE_DIR:-$VERSION_TO_USE/kernel/module/rust}"
COMMON_CONF="$NORDIC_ROOT/common.conf"
LTE_CONF="${STCP_V2_CONF:-$NORDIC_ROOT/lte.conf}"
EXTRA_CONF_FILE="$COMMON_CONF;$LTE_CONF"
MEMORY_DIR="$STCP_PROJECT_ROOT/artifacts/zephyr-memory"

BUILD_DIR="$APP_ROOT/build-lte"
BOARD="nrf9151dk/nrf9151/ns"

log()  { printf '[INFO] %s\n' "$*"; }
fail() { printf '[FAIL] %s\n' "$*" >&2; exit 1; }


[[ -d "$VERSION_TO_USE" ]] ||
    fail "Missing STCP version: $VERSION_TO_USE"

[[ -f "$APP_ROOT/CMakeLists.txt" ]] ||
    fail "Missing: $APP_ROOT/CMakeLists.txt"
[[ -f "$APP_ROOT/prj.conf" ]] ||
    fail "Missing: $APP_ROOT/prj.conf"
[[ -f "$CONF_FILE" ]] ||
    fail "Missing LTE config: $CONF_FILE"
[[ -f "$MODULE_V2/CMakeLists.txt" ]] ||
    fail "Missing: $MODULE_V2/CMakeLists.txt"
[[ -f "$MODULE_V2/zephyr/module.yml" ]] ||
    fail "Missing: $MODULE_V2/zephyr/module.yml"
[[ -f "$CANONICAL_CORE/Cargo.toml" ]] ||
    fail "Missing canonical core: $CANONICAL_CORE/Cargo.toml"

export STCP_SHARED_RUST_CORE_DIR="$CANONICAL_CORE"
export STCP_CANONICAL_CORE="$CANONICAL_CORE"
export ZEPHYR_SDK_INSTALL_DIR
MEMORY_FILE="$MEMORY_DIR/lte.txt"

log "STCP root       : $STCP_ROOT"
log "Version         : $VERSION_TO_USE"
log "NORDIC root     : $NORDIC_ROOT"
log "Application     : $APP_ROOT"
log "Module-v2       : $MODULE_V2"
log "Canonical core  : $CANONICAL_CORE"
log "LTE config      : $CONF_FILE"
log "Build directory : $BUILD_DIR"
log "Board           : $BOARD"
log "Memory dir      : $MEMORY_DIR"
log "Memory file     : $MEMORY_FILE"

BUILD_LOG="$(mktemp)"
trap 'rm -f "$BUILD_LOG"' EXIT

west build \
    -p always \
    -d "$BUILD_DIR" \
    -b "$BOARD" \
    "$APP_ROOT" \
    -- \
    "-DZEPHYR_EXTRA_MODULES=$MODULE_V2" \
    "-DEXTRA_CONF_FILE=$EXTRA_CONF_FILE" \
    "-DSTCP_SHARED_RUST_CORE_DIR=$CANONICAL_CORE" \
    "-DSTCP_CANONICAL_CORE=$CANONICAL_CORE" \
    "-DZEPHYR_SDK_INSTALL_DIR=$ZEPHYR_SDK_INSTALL_DIR" \
    2>&1 | tee "$BUILD_LOG"

rc=${PIPESTATUS[0]}
(( rc == 0 )) || exit "$rc"

grep -A4 '^Memory region' "$BUILD_LOG" > "$MEMORY_FILE" || true

CONFIG_FILE="$BUILD_DIR/application/zephyr/.config"
[[ -f "$CONFIG_FILE" ]] || CONFIG_FILE="$BUILD_DIR/zephyr/.config"
[[ -f "$CONFIG_FILE" ]] || fail "Resulting .config not found"

log "LTE Kconfig sanity:"

grep -E \
    '^CONFIG_STCP=|^CONFIG_NETWORKING=|^CONFIG_NET_SOCKETS=|^CONFIG_NRF_MODEM_LIB=|^CONFIG_LTE_LINK_CONTROL=|^CONFIG_NET_NATIVE=|^CONFIG_NET_SOCKETS_OFFLOAD=' \
    "$CONFIG_FILE" || true

grep -q '^CONFIG_NRF_MODEM_LIB=y' "$CONFIG_FILE" ||
    fail "CONFIG_NRF_MODEM_LIB=y missing"

grep -q '^CONFIG_LTE_LINK_CONTROL=y' "$CONFIG_FILE" ||
    fail "CONFIG_LTE_LINK_CONTROL=y missing"

grep -q '^CONFIG_NET_SOCKETS_OFFLOAD=y' "$CONFIG_FILE" ||
    fail "CONFIG_NET_SOCKETS_OFFLOAD=y missing for LTE"

if grep -q '^CONFIG_NET_NATIVE=y' "$CONFIG_FILE"; then
    fail "CONFIG_NET_NATIVE=y must not be enabled for LTE modem offload"
fi

grep -q '^CONFIG_STCP=y' "$CONFIG_FILE" ||
    fail "CONFIG_STCP=y missing"

log "DONE"
