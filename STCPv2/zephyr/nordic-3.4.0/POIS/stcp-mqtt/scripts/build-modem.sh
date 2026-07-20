#!/usr/bin/env bash
set -Eeuo pipefail

PROJECT_DIR="/home/pomo/git/STCP/STCPv2/zephyr/nordic/stcp-module"
VENV_DIR="/home/pomo/git/STCP/STCPv2/zephyr/nordic/.venv"
SDK_DIR="${HOME}/zephyr-sdk-1.0.1"
BUILD_DIR="${PROJECT_DIR}/build-nrf9151"
BOARD="nrf9151dk/nrf9151/ns"
APP="${PROJECT_DIR}/samples/stcp_c_port"
MODEM_CONF="${APP}/nrf9151.conf"

# MODEM include
MODEM_INCLUDE="${PROJECT_DIR}/../nrfxlib/nrf_modem/include"

cd "$PROJECT_DIR"

if [[ ! -f "$VENV_DIR/bin/activate" ]]; then
    echo "Error: Nordic Python environment not found: $VENV_DIR" >&2
    exit 1
fi
# shellcheck disable=SC1091
source "$VENV_DIR/bin/activate"

command -v west >/dev/null 2>&1 || {
    echo "Error: west is not available from $VENV_DIR" >&2
    exit 1
}

[[ -f "$APP/CMakeLists.txt" ]] || {
    echo "Error: application not found: $APP" >&2
    exit 1
}
[[ -f "$MODEM_CONF" ]] || {
    echo "Error: nRF9151 config not found: $MODEM_CONF" >&2
    exit 1
}
[[ -f "$PROJECT_DIR/../nrfxlib/nrf_modem/include/nrf_modem_at.h" ]] || {
    echo "Error: nrf_modem_at.h is missing. Run: cd $PROJECT_DIR/.. && west update nrfxlib" >&2
    exit 1
}
[[ -d "$SDK_DIR" ]] || {
    echo "Error: Zephyr SDK not found: $SDK_DIR" >&2
    exit 1
}

export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
export ZEPHYR_SDK_INSTALL_DIR="$SDK_DIR"

rm -rf "$BUILD_DIR"

west build \
    --no-sysbuild \
    -p always \
    -d "$BUILD_DIR" \
    -b "$BOARD" \
    "$APP" \
    -- \
    -DZEPHYR_EXTRA_MODULES="$PROJECT_DIR" \
    -DZEPHYR_SDK_INSTALL_DIR="$SDK_DIR" \
    -DEXTRA_CONF_FILE="$MODEM_CONF" \
    -DEXTRA_FLAGS="-I${MODEM_INCLUDE}" 

echo
echo "Build complete:"
echo "  $BUILD_DIR/zephyr/zephyr.hex"
