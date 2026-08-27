#!/usr/bin/env bash
set -Eeuo pipefail

APP_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
NORDIC_ROOT="$(cd "$APP_ROOT/.." && pwd -P)"
REPO_ROOT="$(git -C "$APP_ROOT" rev-parse --show-toplevel)"

MODULE_V2="${STCP_V2_MODULE:-$NORDIC_ROOT/module-v2}"
CORE="${STCP_CANONICAL_CORE:-$REPO_ROOT/STCPv2/kernel/module/rust}"
BUILD_DIR="${STCP_MQTT_BUILD_DIR:-$APP_ROOT/build-mqtt}"
BOARD="${STCP_V2_BOARD:-nrf9151dk/nrf9151/ns}"
SHIELD="${STCP_V2_SHIELD:-seeed_w5500}"

[[ -f "$MODULE_V2/CMakeLists.txt" ]] || { echo "[FAIL] Missing module-v2: $MODULE_V2" >&2; exit 2; }
[[ -f "$CORE/Cargo.toml" ]] || { echo "[FAIL] Missing shared Rust core: $CORE" >&2; exit 2; }

export STCP_SHARED_RUST_CORE_DIR="$CORE"
export STCP_CANONICAL_CORE="$CORE"

west build \
    -p always \
    -d "$BUILD_DIR" \
    -b "$BOARD" \
    --shield "$SHIELD" \
    "$APP_ROOT" \
    -- \
    "-DZEPHYR_EXTRA_MODULES=$MODULE_V2" \
    "-DSTCP_SHARED_RUST_CORE_DIR=$CORE" \
    "-DSTCP_CANONICAL_CORE=$CORE"
