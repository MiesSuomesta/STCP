#!/usr/bin/env bash
set -Eeuo pipefail

APP_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
STCP_ROOT="$(cd "$APP_ROOT/.." && pwd -P)"
MODULE="$STCP_ROOT/module-v2"
CORE="$STCP_ROOT/kernel/module/rust"
BUILD="${STCP_V2_BUILD_DIR:-$APP_ROOT/build-v2-clean}"
BOARD="${STCP_V2_BOARD:-nrf9151dk/nrf9151/ns}"
SHIELD="${STCP_V2_SHIELD:-seeed_w5500}"
CONF="${STCP_V2_CONF:-$APP_ROOT/ethernet-v2.conf}"

command -v west >/dev/null 2>&1 || {
    echo "[FAIL] west not found in PATH; activate the NCS Python environment first." >&2
    exit 2
}

WEST_TOPDIR="$(west topdir 2>/dev/null || true)"
ZEPHYR_TREE="${ZEPHYR_BASE:-${WEST_TOPDIR:+$WEST_TOPDIR/zephyr}}"

[[ -f "$APP_ROOT/CMakeLists.txt" ]] || { echo "[FAIL] Missing $APP_ROOT/CMakeLists.txt" >&2; exit 2; }
[[ -f "$APP_ROOT/Kconfig" ]] || { echo "[FAIL] Missing $APP_ROOT/Kconfig" >&2; exit 2; }
[[ -f "$MODULE/zephyr/module.yml" ]] || { echo "[FAIL] Missing module-v2: $MODULE" >&2; exit 2; }
[[ -f "$CORE/Cargo.toml" ]] || { echo "[FAIL] Missing canonical core: $CORE/Cargo.toml" >&2; exit 2; }
[[ -f "$CONF" ]] || { echo "[FAIL] Missing config: $CONF" >&2; exit 2; }

# NCS 3.3.0 W5500 release workaround. This only touches the Zephyr driver and
# saves/restores a .stcp-original copy. Disable with STCP_W5500_POLLING=0.
if [[ "${STCP_W5500_POLLING:-1}" == "1" ]]; then
    if [[ -z "$ZEPHYR_TREE" || ! -f "$ZEPHYR_TREE/drivers/ethernet/eth_w5500.c" ]]; then
        echo "[FAIL] Cannot locate Zephyr W5500 driver. ZEPHYR_BASE='$ZEPHYR_TREE'" >&2
        exit 2
    fi
    echo "[INFO] Applying NCS 3.3.0 W5500 IRQ polling fallback..."
    python3 "$APP_ROOT/scripts/patch-w5500-driver.py" \
        "$ZEPHYR_TREE/drivers/ethernet/eth_w5500.c"
else
    echo "[INFO] W5500 polling fallback disabled (STCP_W5500_POLLING=0)."
fi

export STCP_SHARED_RUST_CORE_DIR="$CORE"

printf '[INFO] STCP root : %s\n' "$STCP_ROOT"
printf '[INFO] App       : %s\n' "$APP_ROOT"
printf '[INFO] Module-v2 : %s\n' "$MODULE"
printf '[INFO] Rust core : %s\n' "$CORE"
printf '[INFO] Build     : %s\n' "$BUILD"
printf '[INFO] Board     : %s\n' "$BOARD"
printf '[INFO] Shield    : %s\n' "$SHIELD"
printf '[INFO] Config    : %s\n' "$CONF"

west build -p always \
    -d "$BUILD" \
    -b "$BOARD" \
    --shield "$SHIELD" \
    "$APP_ROOT" \
    -- \
    -DZEPHYR_EXTRA_MODULES="$MODULE" \
    -DCONF_FILE="$CONF"

CONFIG="$BUILD/application/zephyr/.config"
if [[ ! -f "$CONFIG" ]]; then
    CONFIG="$BUILD/zephyr/.config"
fi

[[ -f "$CONFIG" ]] || { echo "[FAIL] Build completed but .config not found." >&2; exit 3; }

echo "[INFO] v2 sanity check: $CONFIG"
grep -E '^CONFIG_STCP_V2=|^CONFIG_NET_NATIVE=|NET_SOCKETS_OFFLOAD' "$CONFIG" || true

if ! grep -qx 'CONFIG_STCP_V2=y' "$CONFIG"; then
    echo "[FAIL] CONFIG_STCP_V2 is not enabled." >&2
    exit 3
fi
if grep -qx 'CONFIG_NET_SOCKETS_OFFLOAD=y' "$CONFIG"; then
    echo "[FAIL] Global NET_SOCKETS_OFFLOAD unexpectedly enabled." >&2
    exit 3
fi
if ! grep -qx 'CONFIG_NET_NATIVE=y' "$CONFIG"; then
    echo "[FAIL] Native network stack is not enabled." >&2
    exit 3
fi

echo "[OK] STCPv2 clean Zephyr build ready: $BUILD"
