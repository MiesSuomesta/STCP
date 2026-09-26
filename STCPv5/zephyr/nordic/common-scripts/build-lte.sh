VERSION_TO_USE="$(readlink -f "$STCP_ROOT")"
NORDIC_ROOT="$VERSION_TO_USE/zephyr/nordic"
APP_ROOT="$NORDIC_ROOT/application"
MODULE_V2="$NORDIC_ROOT/module-v2"
CANONICAL_CORE="$VERSION_TO_USE/kernel/module/rust"

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
CONF_FILE="$NORDIC_ROOT/lte.conf"

BUILD_DIR="$APP_ROOT/build-lte"
BOARD="nrf9151dk/nrf9151/ns"

log()  { printf '[INFO] %s\n' "$*"; }
fail() { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

[[ -d "$VERSION_TO_USE" ]] ||
    fail "Missing STCP version: $VERSION_TO_USE"

[[ -f "$APP_ROOT/CMakeLists.txt" ]] ||
    fail "Missing application: $APP_ROOT"

[[ -f "$CONF_FILE" ]] ||
    fail "Missing LTE config: $CONF_FILE"

[[ -f "$MODULE_V2/zephyr/module.yml" ]] ||
    fail "Missing STCP Zephyr module: $MODULE_V2"

[[ -f "$CANONICAL_CORE/Cargo.toml" ]] ||
    fail "Missing canonical Rust core: $CANONICAL_CORE"

export STCP_SHARED_RUST_CORE_DIR="$CANONICAL_CORE"
export STCP_CANONICAL_CORE="$CANONICAL_CORE"
export ZEPHYR_SDK_INSTALL_DIR

log "STCP root       : $STCP_ROOT"
log "Version         : $VERSION_TO_USE"
log "NORDIC root     : $NORDIC_ROOT"
log "Application     : $APP_ROOT"
log "Module-v2       : $MODULE_V2"
log "Canonical core  : $CANONICAL_CORE"
log "LTE config      : $CONF_FILE"
log "Build directory : $BUILD_DIR"
log "Board           : $BOARD"

west build \
    -p always \
    -d "$BUILD_DIR" \
    -b "$BOARD" \
    "$APP_ROOT" \
    -- \
    "-DZEPHYR_EXTRA_MODULES=$MODULE_V2" \
    "-DEXTRA_CONF_FILE=$CONF_FILE" \
    "-DSTCP_SHARED_RUST_CORE_DIR=$CANONICAL_CORE" \
    "-DSTCP_CANONICAL_CORE=$CANONICAL_CORE" \
    "-DZEPHYR_SDK_INSTALL_DIR=$ZEPHYR_SDK_INSTALL_DIR"

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
