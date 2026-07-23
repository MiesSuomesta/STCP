#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"

echo "== STCP Clean =="

#
# Kernel buildit
#
find "$ROOT" \
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
    -type d \
    -name target \
    -exec rm -rf {} +

#
# CMake
#
find "$ROOT" \
    -type d \
    -name build \
    -exec rm -rf {} +

find "$ROOT" \
    -name CMakeCache.txt \
    -delete

find "$ROOT" \
    -name CMakeFiles \
    -type d \
    -exec rm -rf {} +

#
# Python
#
find "$ROOT" \
    -type d \
    -name "__pycache__" \
    -exec rm -rf {} +

find "$ROOT" \
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
    \( -name '*~' \
    -o -name '*.swp' \
    -o -name '.DS_Store' \) \
    -delete

echo
echo "Clean complete."
