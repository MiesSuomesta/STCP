#!/usr/bin/env bash
set -Eeuo pipefail

log()  { printf '[INFO] %s\n' "$*"; }
fail() { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
DEFAULT_APP_ROOT="$(cd "$SCRIPT_DIR/.." && pwd -P)"

APP_ROOT="${STCP_V2_APP_ROOT:-$DEFAULT_APP_ROOT}"

if GIT_TOP="$(git -C "$APP_ROOT" rev-parse --show-toplevel 2>/dev/null)"; then
    VERSION_TO_USE="$(readlink -f "$GIT_TOP/version-to-use")"
    DEFAULT_STCP_ROOT="$VERSION_TO_USE/zephyr/nordic"
    DEFAULT_CANONICAL_CORE="$VERSION_TO_USE/kernel/module/rust"
else
    DEFAULT_STCP_ROOT="$(cd "$APP_ROOT/.." && pwd -P)"
    DEFAULT_CANONICAL_CORE="$(readlink -f "/srv/stcp-project/STCP/version-to-use")/kernel/module/rust"
fi

STCP_ROOT="${STCP_ROOT:-$DEFAULT_STCP_ROOT}"
MODULE_V2="${STCP_V2_MODULE:-$STCP_ROOT/module-v2}"
CANONICAL_CORE="${STCP_SHARED_RUST_CORE_DIR:-${STCP_CANONICAL_CORE:-$DEFAULT_CANONICAL_CORE}}"

BUILD_DIR="${STCP_V2_BUILD_DIR:-$APP_ROOT/build-v2-clean}"
BOARD="${STCP_V2_BOARD:-nrf9151dk/nrf9151/ns}"
SHIELD="${STCP_V2_SHIELD:-seeed_w5500}"
CONF_FILE="${STCP_V2_CONF:-$APP_ROOT/ethernet-v2.conf}"

log "Git top         : ${GIT_TOP:-<not in git>}"
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
log "STCP_CANONICAL_CORE=$STCP_CANONICAL_CORE"

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

CONFIG_FILE=""

for candidate in \
    "$BUILD_DIR/$(basename "$APP_ROOT")/zephyr/.config" \
    "$BUILD_DIR/application/zephyr/.config" \
    "$BUILD_DIR/zephyr/.config"
do
    if [[ -f "$candidate" ]]; then
        CONFIG_FILE="$candidate"
        break
    fi
done

if [[ -z "$CONFIG_FILE" ]]; then
    CONFIG_FILE="$(
        find "$BUILD_DIR" -path '*/zephyr/.config' -type f \
            ! -path '*/_sysbuild/*' \
            -print -quit 2>/dev/null || true
    )"
fi

[[ -n "$CONFIG_FILE" && -f "$CONFIG_FILE" ]] ||
    fail "Resulting application .config not found below $BUILD_DIR"

log "Result config   : $CONFIG_FILE"
log "Kconfig sanity:"

grep -E \
    '^CONFIG_STCP_V2=|^CONFIG_STCP=|^CONFIG_NET_NATIVE=|NET_SOCKETS_OFFLOAD|^CONFIG_MQTT_' \
    "$CONFIG_FILE" || true

grep -q '^CONFIG_NET_SOCKETS_OFFLOAD=y' "$CONFIG_FILE" &&
    fail "CONFIG_NET_SOCKETS_OFFLOAD=y leaked into clean-v2 build"

grep -q '^CONFIG_NET_NATIVE=y' "$CONFIG_FILE" ||
    fail "CONFIG_NET_NATIVE=y missing"

log "DONE"
