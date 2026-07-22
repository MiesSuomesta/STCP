#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
KDIR="${KDIR:-}"
CROSS_COMPILE="${CROSS_COMPILE:-aarch64-linux-gnu-}"
JOBS="${JOBS:-$(nproc)}"
LLVM="${LLVM:-0}"

if [[ -z "$KDIR" ]]; then
    echo "Usage: KDIR=/path/to/prepared/raspberry-kernel $0" >&2
    exit 2
fi

if [[ ! -f "$KDIR/Makefile" || ! -f "$KDIR/.config" ]]; then
    echo "Error: KDIR is not a configured kernel build tree: $KDIR" >&2
    echo "Run the Raspberry kernel defconfig and 'make modules_prepare' first." >&2
    exit 1
fi

if ! command -v "${CROSS_COMPILE}gcc" >/dev/null; then
    echo "Error: ${CROSS_COMPILE}gcc not found" >&2
    exit 1
fi

cd "$ROOT"
make \
    KDIR="$KDIR" \
    ARCH=arm64 \
    CROSS_COMPILE="$CROSS_COMPILE" \
    LLVM="$LLVM" \
    V=1 \
    -j"$JOBS" \
    module

make KDIR="$KDIR" ARCH=arm64 CROSS_COMPILE="$CROSS_COMPILE" check-relocations

file stcp.ko
"${CROSS_COMPILE}readelf" -h stcp.ko | grep -E 'Class:|Data:|Machine:'
"${CROSS_COMPILE}readelf" -p .modinfo stcp.ko | grep -E 'vermagic|name=' || true
