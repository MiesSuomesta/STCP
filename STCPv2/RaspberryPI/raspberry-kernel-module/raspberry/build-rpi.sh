#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
KDIR="${KDIR:-/lib/modules/$(uname -r)/build}"
JOBS="${JOBS:-$(nproc)}"

if [[ "$(uname -m)" != "aarch64" ]]; then
    echo "Error: this package is Raspberry Pi ARM64-only; host is $(uname -m)" >&2
    exit 1
fi

if [[ ! -d "$KDIR" ]]; then
    echo "Error: kernel build directory not found: $KDIR" >&2
    echo "Install headers matching kernel $(uname -r)." >&2
    exit 1
fi

kernel_release="$(make -sC "$KDIR" kernelrelease)"
if [[ "$kernel_release" != "$(uname -r)" ]]; then
    echo "Error: headers ($kernel_release) do not match running kernel ($(uname -r))." >&2
    exit 1
fi

cd "$ROOT"
make KDIR="$KDIR" LLVM=1 V=1 -j"$JOBS" module
make check-relocations

file stcp.ko
modinfo stcp.ko | grep -E '^(filename|version|vermagic|name):' || true
