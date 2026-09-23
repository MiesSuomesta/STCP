#!/usr/bin/env bash
set -Eeuo pipefail

log()  { printf '[INFO] %s\n' "$*"; }
fail() { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

APP_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
STCP_ROOT="${STCP_ROOT:-$HOME/zephyr-stcp/stcp}"
MODULE_V2="${STCP_V2_MODULE:-$STCP_ROOT/module-v2}"
CANONICAL_CORE="${STCP_CANONICAL_CORE:-$(readlink -f "$HOME/git/github/STCP/version-to-use")/kernel/module/rust}"
BUILD_DIR="${STCP_P2P_BUILD_DIR:-$APP_ROOT/build}"
BOARD="${STCP_V2_BOARD:-nrf9151dk/nrf9151/ns}"
SHIELD="${STCP_V2_SHIELD:-seeed_w5500}"
CONF_FILE="${STCP_P2P_CONF:-$APP_ROOT/ethernet-p2p.conf}"

[[ -f "$APP_ROOT/CMakeLists.txt" ]] || fail "Missing $APP_ROOT/CMakeLists.txt"
[[ -f "$MODULE_V2/zephyr/module.yml" ]] || fail "Missing $MODULE_V2/zephyr/module.yml"
[[ -f "$CANONICAL_CORE/Cargo.toml" ]] || fail "Missing canonical core $CANONICAL_CORE/Cargo.toml"

log "P2P application : $APP_ROOT"
log "Module-v2       : $MODULE_V2"
log "Canonical core  : $CANONICAL_CORE"
log "Build directory : $BUILD_DIR"

export STCP_SHARED_RUST_CORE_DIR="$CANONICAL_CORE"
export STCP_CANONICAL_CORE="$CANONICAL_CORE"

west build -p always -d "$BUILD_DIR" -b "$BOARD" --shield "$SHIELD" "$APP_ROOT" -- \
    "-DZEPHYR_EXTRA_MODULES=$MODULE_V2" \
    "-DSTCP_SHARED_RUST_CORE_DIR=$CANONICAL_CORE" \
    "-DSTCP_CANONICAL_CORE=$CANONICAL_CORE" \
    "-DEXTRA_CONF_FILE=$CONF_FILE"

CONFIG_FILE="$BUILD_DIR/application/zephyr/.config"
[[ -f "$CONFIG_FILE" ]] || CONFIG_FILE="$BUILD_DIR/zephyr/.config"
[[ -f "$CONFIG_FILE" ]] || fail "Resulting .config not found"

grep -q '^CONFIG_STCP_V2=y' "$CONFIG_FILE" || fail "CONFIG_STCP_V2=y missing"
grep -q '^CONFIG_NET_NATIVE=y' "$CONFIG_FILE" || fail "CONFIG_NET_NATIVE=y missing"
grep -q '^CONFIG_NET_SOCKETS_OFFLOAD=y' "$CONFIG_FILE" && fail "socket offload leaked into P2P build"

log "P2P build DONE"
