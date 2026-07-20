#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
west build --no-sysbuild -p always -d "$ROOT/build-native" \
  -b native_sim/native/64 "$ROOT/samples/stcp_c_port" -- \
  -DZEPHYR_EXTRA_MODULES="$ROOT"
