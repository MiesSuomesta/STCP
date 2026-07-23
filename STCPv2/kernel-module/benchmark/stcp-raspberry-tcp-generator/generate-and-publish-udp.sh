#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
INPUT="${1:?Usage: $0 BENCHMARK_RESULT_DIR [OUTPUT_DIR]}"
OUTPUT="${2:-$ROOT/generated/raspberry-udp}"
python3 "$ROOT/generate_dashboard.py" "$INPUT" "$OUTPUT" \
  --platform "${PLATFORM_NAME:-Raspberry Pi}" \
  --transport udp \
  --title "${PAGE_TITLE:-Raspberry Pi UDP carrier benchmark}" \
  --commit "${GIT_COMMIT:-$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo unknown)}" \
  --kernel "${BENCHMARK_KERNEL:-unknown}" \
  --compiler "${BENCHMARK_COMPILER:-unknown}"
if [[ "${AUTO_PUBLISH_WEB:-0}" == "1" ]]; then
  "$ROOT/publish-udp.sh" "$OUTPUT"
fi
