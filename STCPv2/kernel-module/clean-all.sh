#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

log() { printf '[INFO] %s\n' "$*"; }
ok()  { printf '[ OK ] %s\n' "$*"; }

# Clean only STCP-owned build trees. Never walk or modify kernel/NCS source
# trees such as raspberry-kernel-sources, linux-next or Nordic SDK folders.
MODULE_DIRS=(
  "$ROOT/x86-kernel-module"
  "$ROOT/raspberry-kernel-module"
)

log "Cleaning STCP module build outputs"

for dir in "${MODULE_DIRS[@]}"; do
  [[ -d "$dir" ]] || continue

  if [[ -f "$dir/Makefile" ]]; then
    make -C "$dir" clean >/dev/null 2>&1 || true
  fi

  find "$dir" -depth -type f \
    \( \
      -name '*.o' -o \
      -name '*.o.cmd' -o \
      -name '.*.cmd' -o \
      -name '*.mod' -o \
      -name '*.mod.c' -o \
      -name '*.ko' -o \
      -name 'Module.symvers' -o \
      -name 'modules.order' \
    \) -delete

done

for rust_dir in \
  "$ROOT/common-rust/target" \
  "$ROOT/x86-kernel-module/rust/target" \
  "$ROOT/raspberry-kernel-module/rust/target"; do
  [[ -d "$rust_dir" ]] && rm -rf -- "$rust_dir"
done

# Generated benchmark pages/results are intentionally preserved.
ok "STCP outputs cleaned; kernel source trees and benchmark results untouched"
