#!/usr/bin/env bash
# smoke_test.sh — STCP kernel Rust smoke tests
# Usage:
#   ./smoke_test.sh [OPTIONS]
# Options:
#   -p <project_root>    Root of kernel/stcp project (default: current dir)
#   -k <kmod_dir>        kmod directory relative to project root (default: ./kmod)
#   -t <rust_target>     rust target dir under rust/target (default: kernel_x86_64-stcp-kernel/release)
#   -n                   do not attempt insmod test (safe dry-run)
#   -h                   show help
#
# Exit codes:
#   0  = all checks passed
#   1  = build failed / make error
#   2  = suspicious relocations (GOTPCREL) found
#   3  = forbidden intrinsics found
#   4  = undefined symbols found
#   5  = insmod/dmesg reported relocation error
#   10 = usage / invocation error

set -euo pipefail

# Defaults
PROJECT_ROOT="$(pwd)"
KMOD_DIR="./kmod"
RUST_TARGET_SUBDIR="kernel_x86_64-stcp-kernel/release"
DO_INSMOD=1

# Forbidden/interesting intrinsic symbols to flag
FORBIDDEN_SYMS=(
  "__addtf3" "__divtf3" "__eqtf2" "" "__floatsitf" ""
  "fma" "" "__letf2" "" "" "" ""
  "" "" "__umodti3" "__unordtf2"
)

usage() {
  sed -n '1,120p' <<'EOF'
Usage: ./smoke_test.sh [OPTIONS]

Options:
  -p <project_root>    Root of kernel/stcp project (default: current dir)
  -k <kmod_dir>        kmod directory relative to project root (default: ./kmod)
  -t <rust_target>     rust target dir under rust/target (default: kernel_x86_64-stcp-kernel/release)
  -n                   do not attempt insmod test (safe dry-run)
  -h                   show help

This script will:
 - run `make rust-build` and `make kmod`
 - inspect the produced object/ko for GOTPCREL relocations
 - look for forbidden compiler intrinsics (libgcc/compiler-rt)
 - check for undefined symbols (UND)
 - try to insmod the module and look for "Unknown rela relocation" in dmesg
EOF
  exit 10
}

while getopts "p:k:t:nh" opt; do
  case $opt in
    p) PROJECT_ROOT="$OPTARG" ;;
    k) KMOD_DIR="$OPTARG" ;;
    t) RUST_TARGET_SUBDIR="$OPTARG" ;;
    n) DO_INSMOD=0 ;;
    h) usage ;;
    *) usage ;;
  esac
done

cd "$PROJECT_ROOT" || { echo "Project root not found: $PROJECT_ROOT"; exit 10; }

KMOD_PATH="$PROJECT_ROOT/$KMOD_DIR"
if [ ! -d "$KMOD_PATH" ]; then
  echo "KMOD directory not found: $KMOD_PATH"
  exit 10
fi

# Determine produced files (prefer .ko after make kmod)
KO_CANDIDATES=("$KMOD_PATH"/*.ko)
O_CANDIDATES=("$KMOD_PATH"/*.o)

# Colors for readability
RED=$(printf '\033[0;31m')
GREEN=$(printf '\033[0;32m')
YELLOW=$(printf '\033[0;33m')
NC=$(printf '\033[0m')

echo "${GREEN}== STCP Rust smoke test starting ==${NC}"
echo "Project root: $PROJECT_ROOT"
echo "Kmod dir: $KMOD_PATH"
echo "Rust target subdir: $RUST_TARGET_SUBDIR"
echo ""

# 1) build Rust and kmod
echo "${YELLOW}-- Running build (make rust-build && make kmod)${NC}"
if ! make rust-build; then
  echo "${RED}Build failed: make rust-build${NC}" >&2
  exit 1
fi

if ! make kmod; then
  echo "${RED}Build failed: make kmod${NC}" >&2
  exit 1
fi
echo "${GREEN}Build finished.${NC}"
echo ""

# pick the object/ko to inspect
KO_FILE=""
if compgen -G "$KMOD_PATH/*.ko" > /dev/null; then
  KO_FILE=$(ls -1 "$KMOD_PATH"/*.ko | head -n1)
else
  # fallback to .o produced from rust staticlib link
  if compgen -G "$KMOD_PATH/*.o" > /dev/null; then
    KO_FILE=$(ls -1 "$KMOD_PATH"/*.o | head -n1)
  else
    echo "${RED}No .ko or .o artefact found in $KMOD_PATH${NC}" >&2
    exit 1
  fi
fi

echo "Inspecting: $KO_FILE"
echo ""

# Helper: run readelf -r and extract R_X86_64_GOTPCREL targets
echo "${YELLOW}-- Checking relocations (R_X86_64_GOTPCREL)...${NC}"
RELA_LIST=$(readelf -r "$KO_FILE" 2>/dev/null | awk '/R_X86_64_GOTPCREL/ { print $5 }' || true)

if [ -z "$RELA_LIST" ]; then
  echo "${GREEN}No R_X86_64_GOTPCREL relocations found.${NC}"
else
  echo "${RED}Found R_X86_64_GOTPCREL relocations pointing to:${NC}"
  echo "$RELA_LIST" | sort -u
  # Check for known good symbols:
  echo ""
  echo "Allowed kernel-provided symbols are typically: memcpy, memmove, memset, memcmp, bcmp"
  # Flag forbidden ones
  BAD_FOUND=0
  for s in "${FORBIDDEN_SYMS[@]}"; do
    if echo "$RELA_LIST" | grep -q "^$s$"; then
      echo "${RED}  -> Forbidden intrinsic present: $s${NC}"
      BAD_FOUND=1
    fi
  done
  # Also flag any unexpected names (not the allowed set)
  for sym in $(echo "$RELA_LIST" | sort -u); do
    case "$sym" in
      memcpy|memmove|memset|memcmp|bcmp)
        # ok
        ;;
      *)
        # if not in forbidden list, still report
        if ! printf '%s\n' "${FORBIDDEN_SYMS[@]}" | grep -qx "$sym"; then
          echo "${YELLOW}  -> Unexpected relocation symbol: $sym${NC}"
        fi
        ;;
    esac
  done

  if [ "$BAD_FOUND" -ne 0 ]; then
    echo "${RED}ERROR: forbidden intrinsics present in relocations.${NC}"
    exit 2
  else
    echo "${GREEN}Relocations OK (no forbidden intrinsics).${NC}"
  fi
fi

echo ""
# 2) Check undefined symbols in the module (.ko is preferred)
echo "${YELLOW}-- Checking undefined symbols (UND) in $KO_FILE ...${NC}"
UND_LIST=$(readelf -s "$KO_FILE" 2>/dev/null | awk '$4=="UND" {print $8}' | sort -u || true)

if [ -z "$UND_LIST" ]; then
  echo "${GREEN}No undefined symbols (UND) in $KO_FILE.${NC}"
else
  echo "${RED}Undefined symbols found:${NC}"
  echo "$UND_LIST"
  echo "${RED}ERROR: undefined symbols present in module. Investigate!${NC}"
  exit 4
fi

# 3) Double-check for any forbidden symbol names anywhere in binary
echo ""
echo "${YELLOW}-- Grepping for forbidden intrinsics names in binary text...${NC}"
BIN_TEXT=$(strings "$KO_FILE" || true)
FORB_FOUND=0
for s in "${FORBIDDEN_SYMS[@]}"; do
  if echo "$BIN_TEXT" | grep -q "$s"; then
    echo "${RED}Found forbidden symbol text: $s${NC}"
    FORB_FOUND=1
  fi
done
if [ "$FORB_FOUND" -ne 0 ]; then
  echo "${RED}ERROR: Forbidden intrinsic names appear in binary text${NC}"
  exit 3
else
  echo "${GREEN}No forbidden intrinsic names found in binary text.${NC}"
fi

# 4) Optional: try to insmod module and check dmesg for relocation errors
# 4) Optional: try to insmod module and check dmesg for relocation errors
if [ "$DO_INSMOD" -eq 1 ]; then
  echo ""
  echo "${YELLOW}-- Insmod test (requires root). Will attempt to insmod then remove module.${NC}"
  if [ "$EUID" -ne 0 ]; then
    echo "${YELLOW}Not running as root: using sudo for insmod/rmmod/dmesg${NC}"
    SUDO="sudo"
  else
    SUDO=""
  fi

  MOD_NAME="$(basename "$KO_FILE" .ko)"

  # Yritä poistaa mahdollinen aiempi instanssi hiljaa
  $SUDO rmmod "$MOD_NAME" 2>/dev/null || true

  # Tyhjennä dmesg vain meidän testin ajaksi
  $SUDO dmesg -C 2>/dev/null || true

  echo "Trying to insert module: $KO_FILE"
  if ! $SUDO insmod "$KO_FILE"; then
    echo "${RED}insmod failed (command returned non-zero). Check dmesg output below.${NC}"
  fi

  sleep 0.5
  DM_OUT=$($SUDO dmesg -T | tail -n 50 || true)
  echo "---- dmesg (last lines) ----"
  echo "$DM_OUT"
  echo "----------------------------"

  if echo "$DM_OUT" | grep -qi "Unknown rela relocation"; then
    echo "${RED}Kernel reported Unknown rela relocation (bad).${NC}"
    $SUDO rmmod "$MOD_NAME" 2>/dev/null || true
    exit 5
  fi

  # Poista moduuli lopuksi jos se on ladattuna
  $SUDO rmmod "$MOD_NAME" 2>/dev/null || true

  echo "${GREEN}Insmod test passed (no relocation errors observed in dmesg).${NC}"
else
  echo "${YELLOW}Skipped insmod test (dry-run).${NC}"
fi

echo ""
echo "${GREEN}== Smoke test PASSED: module looks clean for GOT/relocations/UNDEFs ==${NC}"
exit 0
