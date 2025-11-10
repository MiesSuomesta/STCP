#!/usr/bin/env bash
set -euo pipefail
KDIR="${KDIR:-/lib/modules/$(uname -r)/build}"
echo "[i] Building STCP module against: $KDIR"
make -C "$(dirname "$0")/.." KDIR="$KDIR" all
echo "[i] Done."
echo "[i] You can insert with: sudo insmod kernel/stcp_rust.ko"
