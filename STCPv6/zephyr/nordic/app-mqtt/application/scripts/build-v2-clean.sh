#!/usr/bin/env bash
set -Eeuo pipefail

log()  { printf '[INFO] %s\n' "$*"; }
fail() { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

GIT_TOP="$(git rev-parse --show-toplevel)" || fail "Run inside the STCP Git repository"
VERSION_TO_USE="$(readlink -f "$GIT_TOP/version-to-use")"
[[ -d "$VERSION_TO_USE" ]] || fail "Missing $GIT_TOP/version-to-use"
STCP_ROOT="${STCP_ROOT:-$VERSION_TO_USE/zephyr/nordic}"
APP_ROOT="${STCP_V2_APP_ROOT:-$STCP_ROOT/app-mqtt}"
MODULE_V2="${STCP_V2_MODULE:-$STCP_ROOT/module-v2}"
CANONICAL_CORE="${STCP_SHARED_RUST_CORE_DIR:-${STCP_CANONICAL_CORE:-$VERSION_TO_USE/kernel/module/rust}}"

BUILD_DIR="${STCP_V2_BUILD_DIR:-$APP_ROOT/build-v2-clean}"
BOARD="${STCP_V2_BOARD:-nrf9151dk/nrf9151/ns}"
SHIELD="${STCP_V2_SHIELD:-seeed_w5500}"
CONF_FILE="${STCP_V2_CONF:-$APP_ROOT/ethernet-v2.conf}"

log "STCP root       : $STCP_ROOT"
log "Application     : $APP_ROOT"
log "Module-v2       : $MODULE_V2"
log "Canonical core  : $CANONICAL_CORE"
log "Build directory : $BUILD_DIR"
log "Board           : $BOARD"
log "Shield          : $SHIELD"
log "Config          : $CONF_FILE"

[[ -f "$APP_ROOT/CMakeLists.txt" ]] || fail "Missing: $APP_ROOT/CMakeLists.txt"
[[ -f "$APP_ROOT/prj.conf" ]] || fail "Missing: $APP_ROOT/prj.conf"
[[ -f "$MODULE_V2/CMakeLists.txt" ]] || fail "Missing: $MODULE_V2/CMakeLists.txt"
[[ -f "$MODULE_V2/zephyr/module.yml" ]] || fail "Missing: $MODULE_V2/zephyr/module.yml"
[[ -f "$CANONICAL_CORE/Cargo.toml" ]] || fail "Missing canonical core: $CANONICAL_CORE/Cargo.toml"
[[ -f "$CONF_FILE" ]] || fail "Missing: $CONF_FILE"

export STCP_SHARED_RUST_CORE_DIR="$CANONICAL_CORE"
export STCP_CANONICAL_CORE="$CANONICAL_CORE"

log "STCP_SHARED_RUST_CORE_DIR=$STCP_SHARED_RUST_CORE_DIR"

west build \
    -p always \
    -d "$BUILD_DIR" \
    -b "$BOARD" \
    --shield "$SHIELD" \
    "$APP_ROOT" \
    -- \
    "-DZEPHYR_EXTRA_MODULES=$MODULE_V2" \
    "-DSTCP_SHARED_RUST_CORE_DIR=$CANONICAL_CORE" \
    "-DSTCP_CANONICAL_CORE=$CANONICAL_CORE" \
    "-DEXTRA_CONF_FILE=$CONF_FILE"

CONFIG_FILE="$BUILD_DIR/application/zephyr/.config"
[[ -f "$CONFIG_FILE" ]] || CONFIG_FILE="$BUILD_DIR/zephyr/.config"
[[ -f "$CONFIG_FILE" ]] || fail "Resulting .config not found"

log "Kconfig sanity:"
grep -E '^CONFIG_STCP_V2=|^CONFIG_STCP=|^CONFIG_NET_NATIVE=|NET_SOCKETS_OFFLOAD' \
    "$CONFIG_FILE" || true

grep -q '^CONFIG_NET_SOCKETS_OFFLOAD=y' "$CONFIG_FILE" &&
    fail "CONFIG_NET_SOCKETS_OFFLOAD=y leaked into clean-v2 build"

grep -q '^CONFIG_NET_NATIVE=y' "$CONFIG_FILE" ||
    fail "CONFIG_NET_NATIVE=y missing"

log "DONE"
