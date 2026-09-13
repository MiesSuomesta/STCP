#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
required=(
  kernel/module/Makefile
  kernel/module/Kbuild
  zephyr/nordic/module/CMakeLists.txt
  zephyr/nordic/application/CMakeLists.txt
  tests/kernel/testing
  tests/kernel/stress
  tests/benchmark/raspberrypi
  tests/benchmark/zephyr
  tests/robot
)
for rel in "${required[@]}"; do
  [[ -e "$ROOT/$rel" ]] || { echo "[FAIL] missing: $rel" >&2; exit 1; }
done
find "$ROOT" -type f -name '*.sh' -print0 | xargs -0 -n1 bash -n
echo "[OK] STCPv2 refactored layout looks consistent"
