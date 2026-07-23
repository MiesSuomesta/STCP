#!/usr/bin/env bash
set -Eeuo pipefail

export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:${PATH:-}"

# Build and deploy STCP kernel modules for both:
#   - local x86_64 host
#   - remote Raspberry Pi ARM64
#
# Expected layout:
#   project-root/
#   ├── common-rust/
#   ├── x86-kernel-module/
#   ├── raspberry-kernel-module/
#   ├── raspberry-kernel-sources/
#   └── build-and-deploy.sh
#
# Typical usage:
#   RPI_HOST=pi@192.168.1.199 ./build-and-deploy.sh all
#
# Modes:
#   all            build + install x86 and Raspberry
#   build          build both, install neither
#   x86            build + install x86 only
#   rpi            build + install Raspberry only
#   install        install already-built modules only
#   clean          clean both module builds and Rust targets
#
# Environment:
#   RPI_HOST=pi@192.168.1.199
#   RPI_SSH_OPTS="-o StrictHostKeyChecking=accept-new"
#   RPI_REMOTE_DIR=/tmp/stcp-deploy
#   RPI_MODULE_PATH=/lib/modules/$(uname -r)/extra/stcp.ko
#   RPI_KDIR=/absolute/path/to/raspberry-kernel-sources
#   X86_KDIR=/lib/modules/$(uname -r)/build
#   CROSS_COMPILE=aarch64-linux-gnu-
#   JOBS=$(nproc)
#   CARRIER_DEBUG=1
#   STOP_BENCHMARKS=1
#   DRY_RUN=1

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
MODE="${1:-all}"

COMMON_RUST="${COMMON_RUST:-$SCRIPT_DIR/common-rust}"
X86_DIR="${X86_DIR:-$SCRIPT_DIR/x86-kernel-module}"
RPI_DIR="${RPI_DIR:-$SCRIPT_DIR/raspberry-kernel-module}"
RPI_KDIR="${RPI_KDIR:-$SCRIPT_DIR/raspberry-kernel-sources}"

X86_KDIR="${X86_KDIR:-/lib/modules/$(uname -r)/build}"
CROSS_COMPILE="${CROSS_COMPILE:-aarch64-linux-gnu-}"
JOBS="${JOBS:-$(nproc)}"

RPI_HOST="${RPI_HOST:-pi@192.168.1.199}"
RPI_SSH_OPTS="${RPI_SSH_OPTS:--o StrictHostKeyChecking=accept-new}"
RPI_REMOTE_DIR="${RPI_REMOTE_DIR:-/tmp/stcp-deploy}"
RPI_MODULE_PATH="${RPI_MODULE_PATH:-}"
CARRIER_DEBUG="${CARRIER_DEBUG:-1}"
STOP_BENCHMARKS="${STOP_BENCHMARKS:-1}"
DRY_RUN="${DRY_RUN:-0}"

log() {
    printf '[INFO] %s\n' "$*"
}

ok() {
    printf '[ OK ] %s\n' "$*"
}

die() {
    printf '[FAIL] %s\n' "$*" >&2
    exit 1
}

run() {
    if [[ "$DRY_RUN" == "1" ]]; then
        printf '+'
        printf ' %q' "$@"
        printf '\n'
        return 0
    fi
    "$@"
}

find_cmd() {
    local name="$1"
    local candidate

    if candidate="$(command -v "$name" 2>/dev/null)"; then
        printf '%s
' "$candidate"
        return 0
    fi

    for candidate in         "/usr/local/sbin/$name"         "/usr/local/bin/$name"         "/usr/sbin/$name"         "/usr/bin/$name"         "/sbin/$name"         "/bin/$name"; do
        if [[ -x "$candidate" ]]; then
            printf '%s
' "$candidate"
            return 0
        fi
    done

    return 1
}

need_cmd() {
    local name="$1"
    local path

    path="$(find_cmd "$name")" || die "Missing command: $name"
    printf '[ OK ] %-24s %s
' "$name" "$path"
}

check_layout() {
    [[ -d "$COMMON_RUST" ]] || die "Missing common Rust tree: $COMMON_RUST"
    [[ -f "$COMMON_RUST/Cargo.toml" ]] || die "Missing: $COMMON_RUST/Cargo.toml"
    [[ -f "$X86_DIR/Makefile" ]] || die "Missing x86 module Makefile: $X86_DIR"
    [[ -f "$RPI_DIR/Makefile" ]] || die "Missing Raspberry module Makefile: $RPI_DIR"
}

check_tools() {
    for cmd in make rustup ssh scp file modinfo insmod rmmod lsmod dmesg sudo; do
        need_cmd "$cmd"
    done

    need_cmd "${CROSS_COMPILE}gcc"
    need_cmd "${CROSS_COMPILE}ld"
    need_cmd "${CROSS_COMPILE}ar"
    need_cmd "${CROSS_COMPILE}readelf"

    rustup toolchain list | grep -q '^nightly' ||
        die "Rust nightly missing: rustup toolchain install nightly --component rust-src"
}

ensure_cleanable_rust() {
    rm -f \
        "$X86_DIR/src/rust_core.o" \
        "$X86_DIR/src/.rust_core.o.cmd" \
        "$RPI_DIR/src/rust_core.o" \
        "$RPI_DIR/src/.rust_core.o.cmd"
}

clean_x86() {
    log "Cleaning x86 build"
    run make -C "$X86_DIR" \
        KDIR="$X86_KDIR" \
        ARCH=x86_64 \
        CROSS_COMPILE= \
        clean || true

    run rm -rf "$COMMON_RUST/target/x86_64-unknown-none"
    run rm -f "$X86_DIR/stcp.ko" "$X86_DIR/stcp.o"
}

clean_rpi() {
    log "Cleaning Raspberry build"
    run make -C "$RPI_DIR" \
        KDIR="$RPI_KDIR" \
        ARCH=arm64 \
        CROSS_COMPILE="$CROSS_COMPILE" \
        clean || true

    run rm -rf "$COMMON_RUST/target/aarch64-unknown-none"
    run rm -f "$RPI_DIR/stcp.ko" "$RPI_DIR/stcp.o"
}

build_x86() {
    [[ -d "$X86_KDIR" ]] || die "x86 kernel build tree missing: $X86_KDIR"

    clean_x86
    ensure_cleanable_rust

    log "Building x86 STCP module"
    run make -C "$X86_DIR" \
        KDIR="$X86_KDIR" \
        ARCH=x86_64 \
        CROSS_COMPILE= \
        RUST_DIR="$COMMON_RUST" \
        -j"$JOBS" module

    [[ "$DRY_RUN" == "1" ]] && return 0

    [[ -f "$X86_DIR/stcp.ko" ]] || die "x86 stcp.ko was not produced"

    file "$X86_DIR/stcp.ko" | grep -Eq 'x86-64|x86_64' ||
        die "x86 module has wrong architecture"

    modinfo "$X86_DIR/stcp.ko" | grep -q "vermagic:.*$(uname -r)" ||
        die "x86 module vermagic does not match $(uname -r)"

    ok "x86 module built: $X86_DIR/stcp.ko"
}

build_rpi() {
    [[ -d "$RPI_KDIR" ]] || die "Raspberry kernel build tree missing: $RPI_KDIR"
    [[ -f "$RPI_KDIR/Module.symvers" ]] ||
        die "Raspberry Module.symvers missing: build the Raspberry kernel first"

    clean_rpi
    ensure_cleanable_rust

    log "Building Raspberry ARM64 STCP module"
    run make -C "$RPI_DIR" \
        KDIR="$RPI_KDIR" \
        ARCH=arm64 \
        CROSS_COMPILE="$CROSS_COMPILE" \
        RUST_DIR="$COMMON_RUST" \
        -j"$JOBS" module

    [[ "$DRY_RUN" == "1" ]] && return 0

    [[ -f "$RPI_DIR/stcp.ko" ]] || die "Raspberry stcp.ko was not produced"

    "${CROSS_COMPILE}readelf" -h "$RPI_DIR/stcp.ko" |
        grep -q 'Machine:.*AArch64' ||
        die "Raspberry module has wrong architecture"

    ok "Raspberry module built: $RPI_DIR/stcp.ko"
}

stop_local_users() {
    if [[ "$STOP_BENCHMARKS" == "1" ]]; then
	run pkill -f -- '[b]enchmark_server\.py' 2>/dev/null || true
	run pkill -f -- '[b]enchmark_client\.py' 2>/dev/null || true
    fi
}

install_x86() {
    local module="$X86_DIR/stcp.ko"
    local module_hash
    local insmod_args=()
    local modinfo_bin
    local insmod_bin
    local rmmod_bin

    [[ -f "$module" ]] || die "Missing x86 module: $module"

    modinfo_bin="$(find_cmd modinfo)" || die "modinfo not found"
    insmod_bin="$(find_cmd insmod)" || die "insmod not found"
    rmmod_bin="$(find_cmd rmmod)" || die "rmmod not found"

    log "Installing x86 module: $module"
    module_hash="$(sha256sum "$module" | awk '{print $1}')"
    log "x86 module sha256=$module_hash"

    stop_local_users

    if grep -q '^stcp ' /proc/modules 2>/dev/null; then
        log "Removing currently loaded x86 STCP module"

        if ! run sudo "$rmmod_bin" stcp; then
            echo
            echo "Loaded module:"
            grep '^stcp ' /proc/modules || true
            echo
            echo "Recent kernel log:"
            sudo dmesg | tail -n 120 || true
            die "Could not unload old x86 STCP module"
        fi
    fi

    if "$modinfo_bin" "$module" | grep -q '^parm:.*carrier_debug'; then
        insmod_args+=("carrier_debug=$CARRIER_DEBUG")
    fi

    log "Loading x86 module with: $insmod_bin $module ${insmod_args[*]:-}"

    if ! run sudo "$insmod_bin" "$module" "${insmod_args[@]}"; then
        echo
        echo "Module information:"
        "$modinfo_bin" "$module" |
            grep -E '^(filename|vermagic|depends|parm):' || true
        echo
        echo "Running kernel:"
        uname -a
        echo
        echo "Recent kernel log:"
        sudo dmesg | tail -n 160 || true
        die "insmod failed for x86 STCP module"
    fi

    [[ "$DRY_RUN" == "1" ]] && return 0

    sleep 0.2

    if ! grep -q '^stcp ' /proc/modules 2>/dev/null; then
        echo
        echo "Recent kernel log:"
        sudo dmesg | tail -n 160 || true
        die "x86 STCP module is not present in /proc/modules after insmod"
    fi

    ok "x86 module loaded"

    if [[ -e /sys/module/stcp/parameters/carrier_debug ]]; then
        log "x86 carrier_debug=$(cat /sys/module/stcp/parameters/carrier_debug)"
    fi

    sudo dmesg | grep -i stcp | tail -n 50 || true
}

remote_ssh() {
    # shellcheck disable=SC2086
    run ssh -tt $RPI_SSH_OPTS "$RPI_HOST" "$@"
}

copy_rpi_module() {
    [[ -f "$RPI_DIR/stcp.ko" ]] || die "Missing Raspberry module: $RPI_DIR/stcp.ko"

    log "Copying Raspberry module to $RPI_HOST"

    # shellcheck disable=SC2086
    run ssh $RPI_SSH_OPTS "$RPI_HOST" "mkdir -p '$RPI_REMOTE_DIR'"

    # shellcheck disable=SC2086
    run scp $RPI_SSH_OPTS \
        "$RPI_DIR/stcp.ko" \
        "$RPI_HOST:$RPI_REMOTE_DIR/stcp.ko"
}

install_rpi() {
    copy_rpi_module

    log "Installing Raspberry module"

    local remote_script
    remote_script=$(cat <<EOF
set -Eeuo pipefail
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:\${PATH:-}"

MODULE="$RPI_REMOTE_DIR/stcp.ko"
MODINFO="\$(command -v modinfo || echo /usr/sbin/modinfo)"
INSMOD="\$(command -v insmod || echo /usr/sbin/insmod)"
MODPROBE="\$(command -v modprobe || echo /usr/sbin/modprobe)"
RMMOD="\$(command -v rmmod || echo /usr/sbin/rmmod)"

[[ -x "\$MODINFO" ]] || { echo "[FAIL] modinfo not found"; exit 1; }
[[ -x "\$INSMOD" ]] || { echo "[FAIL] insmod not found"; exit 1; }
[[ -x "\$MODPROBE" ]] || { echo "[FAIL] modprobe not found"; exit 1; }
[[ -x "\$RMMOD" ]] || { echo "[FAIL] rmmod not found"; exit 1; }
[[ -f "\$MODULE" ]] || { echo "[FAIL] module not found: \$MODULE"; exit 1; }

if [[ "$STOP_BENCHMARKS" == "1" ]]; then
    pkill -f -- '[b]enchmark_server\.py' 2>/dev/null || true
    pkill -f -- '[b]enchmark_client\.py' 2>/dev/null || true
fi

if grep -q '^stcp ' /proc/modules 2>/dev/null; then
    echo "[INFO] Removing old Raspberry STCP module"
    if ! sudo "\$RMMOD" stcp; then
        sudo dmesg | tail -n 120 || true
        exit 1
    fi
fi

ARGS=()
if "\$MODINFO" "\$MODULE" | grep -q '^parm:.*carrier_debug'; then
    ARGS+=("carrier_debug=$CARRIER_DEBUG")
fi

KREL=\$(uname -r)
DEST="/lib/modules/\$KREL/extra/stcp.ko"

echo "[INFO] Installing Raspberry module to: \$DEST"
sudo mkdir -p "\$(dirname "\$DEST")"
sudo install -m 0644 "\$MODULE" "\$DEST"
sudo /usr/sbin/depmod -a "\$KREL"

echo "[INFO] Loading Raspberry module with modprobe: stcp \${ARGS[*]:-}"
if ! sudo "\$MODPROBE" stcp "\${ARGS[@]}"; then
    "\$MODINFO" stcp | grep -E '^(filename|vermagic|depends|parm):' || true
    uname -a
    sudo dmesg | tail -n 160 || true
    exit 1
fi

sleep 0.2

if ! grep -q '^stcp ' /proc/modules 2>/dev/null; then
    sudo dmesg | tail -n 160 || true
    echo "[FAIL] Raspberry STCP module is not present in /proc/modules"
    exit 1
fi

echo "[ OK ] Raspberry module loaded"
if [[ -e /sys/module/stcp/parameters/carrier_debug ]]; then
    echo "[INFO] Raspberry carrier_debug=\$(cat /sys/module/stcp/parameters/carrier_debug)"
fi

sudo dmesg | grep -i stcp | tail -n 60 || true
EOF
)

    remote_ssh "bash -lc $(printf '%q' "$remote_script")"

    ok "Raspberry module loaded"
}

install_rpi_persistent() {
    copy_rpi_module

    log "Installing Raspberry module persistently"

    local remote_script
    remote_script=$(cat <<EOF
set -Eeuo pipefail
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:\${PATH:-}"

KREL=\$(uname -r)
DEST="\${RPI_MODULE_PATH:-/lib/modules/\$KREL/extra/stcp.ko}"

sudo mkdir -p "\$(dirname "\$DEST")"
sudo install -m 0644 "$RPI_REMOTE_DIR/stcp.ko" "\$DEST"
sudo /usr/sbin/depmod -a "\$KREL"

echo "Installed: \$DEST"
EOF
)

    remote_ssh "RPI_MODULE_PATH=$(printf '%q' "$RPI_MODULE_PATH") bash -lc $(printf '%q' "$remote_script")"
}

summary() {
    echo
    echo "== Deployment summary =="
    echo "Common Rust: $COMMON_RUST"
    echo "x86 module:  $X86_DIR/stcp.ko"
    echo "RPI module:  $RPI_DIR/stcp.ko"
    echo "RPI host:    $RPI_HOST"
}

check_layout
check_tools

case "$MODE" in
    all)
        build_x86
        build_rpi
        install_x86
        install_rpi
        ;;
    build)
        build_x86
        build_rpi
        ;;
    x86)
        build_x86
        install_x86
        ;;
    rpi)
        build_rpi
        install_rpi
        ;;
    install)
        install_x86
        install_rpi
        ;;
    install-rpi-persistent)
        install_rpi_persistent
        ;;
    clean)
        clean_x86
        clean_rpi
        ;;
    *)
        die "Unknown mode '$MODE'. Use: all|build|x86|rpi|install|install-rpi-persistent|clean"
        ;;
esac

summary
