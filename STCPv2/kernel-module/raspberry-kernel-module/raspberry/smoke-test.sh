#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
MODULE="${MODULE:-$ROOT/stcp.ko}"
SUDO="${SUDO:-sudo}"

cleanup() {
    if grep -q '^stcp ' /proc/modules 2>/dev/null; then
        SUDO="$SUDO" MODULE=stcp "$ROOT/scripts/graceful-unload.sh" || true
    fi
}
trap cleanup EXIT INT TERM

[[ "$(uname -m)" == "aarch64" ]] || { echo "ARM64 host required" >&2; exit 1; }
[[ -f "$MODULE" ]] || { echo "Module not found: $MODULE" >&2; exit 1; }

module_arch="$(file -b "$MODULE")"
[[ "$module_arch" == *"ARM aarch64"* ]] || {
    echo "Module is not AArch64: $module_arch" >&2
    exit 1
}

cleanup
$SUDO modprobe libcurve25519
$SUDO modprobe libchacha20poly1305
$SUDO insmod "$MODULE"
grep '^stcp ' /proc/modules
$SUDO dmesg | tail -n 80
cleanup
trap - EXIT INT TERM
echo "Raspberry Pi ARM64 module smoke test PASS"
