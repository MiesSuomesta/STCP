#!/usr/bin/env bash
set -u

KDIR="${KDIR:-/lib/modules/$(uname -r)/build}"
failed=0
check() { if eval "$2"; then printf '[ OK ] %-28s %s\n' "$1" "$3"; else printf '[FAIL] %-28s %s\n' "$1" "$3"; failed=1; fi; }

check "Architecture" '[[ "$(uname -m)" == "aarch64" ]]' "$(uname -m)"
check "Kernel build tree" '[[ -d "$KDIR" ]]' "$KDIR"
check "Rustup" 'command -v rustup >/dev/null' "$(command -v rustup 2>/dev/null || echo missing)"
check "LLVM ar" 'command -v llvm-ar >/dev/null' "$(command -v llvm-ar 2>/dev/null || echo missing)"
check "LLD" 'command -v ld.lld >/dev/null' "$(command -v ld.lld 2>/dev/null || echo missing)"
check "Clang" 'command -v clang >/dev/null' "$(command -v clang 2>/dev/null || echo missing)"

if [[ -d "$KDIR" ]]; then
    kr="$(make -sC "$KDIR" kernelrelease 2>/dev/null || echo unknown)"
    check "Header/kernel match" '[[ "$kr" == "$(uname -r)" ]]' "headers=$kr running=$(uname -r)"
fi

exit "$failed"
