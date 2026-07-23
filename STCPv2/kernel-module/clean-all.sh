#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"

echo "== STCP Clean =="

# Hakemistot, joihin EI kosketa
PRUNE=(
    \( \
        -path "$ROOT/kernel-module/raspberry-kernel-sources" \
        -o -path "$ROOT/kernel-module/linux-kernel-sources" \
        -o -path "$ROOT/kernel-module/linux-next" \
    \) -prune -o
)

#
# Kernel-moduulin buildituotteet (mutta EI kernelin lähdepuussa)
#
find "$ROOT" \
    "${PRUNE[@]}" \
    \( -name '*.o' \
    -o -name '*.ko' \
    -o -name '*.mod' \
    -o -name '*.mod.c' \
    -o -name '*.mod.o' \
    -o -name '*.symvers' \
    -o -name '*.order' \
    -o -name '*.cmd' \
    -o -name '*.a' \
    -o -name '*.lst' \
    -o -name '*.su' \
    -o -name '*.gcno' \
    -o -name '*.gcda' \
    \) \
    -delete

find "$ROOT" \
    "${PRUNE[@]}" \
    -type d \
    \( -name '.tmp_versions' \
    -o -name '.rust-objects' \
    -o -name '.cache' \
    \) \
    -exec rm -rf {} +

#
# Rust
#
find "$ROOT" \
    "${PRUNE[@]}" \
    -type d \
    -name target \
    -exec rm -rf {} +

#
# CMake
#
find "$ROOT" \
    "${PRUNE[@]}" \
    -type d \
    -name build \
    -exec rm -rf {} +

find "$ROOT" \
    "${PRUNE[@]}" \
    -name CMakeCache.txt \
    -delete

find "$ROOT" \
    "${PRUNE[@]}" \
    -type d \
    -name CMakeFiles \
    -exec rm -rf {} +

#
# Python
#
find "$ROOT" \
    "${PRUNE[@]}" \
    -type d \
    -name "__pycache__" \
    -exec rm -rf {} +

find "$ROOT" \
    "${PRUNE[@]}" \
    \( -name "*.pyc" \
    -o -name "*.pyo" \
    -o -name "*.pyd" \) \
    -delete

#
# Benchmarkit
#
rm -rf "$ROOT/kernel-module/benchmark/results"

#
# perf
#
sudo rm -f /tmp/stcp-perf-*.csv 2>/dev/null || true
sudo rm -f /tmp/stcp-perf-*.log 2>/dev/null || true

#
# Editorit
#
find "$ROOT" \
    "${PRUNE[@]}" \
    \( -name '*~' \
    -o -name '*.swp' \
    -o -name '.DS_Store' \) \
    -delete

echo
echo "Clean complete."
