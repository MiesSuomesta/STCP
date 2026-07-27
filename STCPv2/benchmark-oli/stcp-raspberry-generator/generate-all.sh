#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
RESULTS_ROOT="${RESULTS_ROOT:-$ROOT/../results}"
TCP_INPUT="${1:-${TCP_RESULTS_DIR:-}}"
UDP_INPUT="${2:-${UDP_RESULTS_DIR:-}}"
OUTPUT="${3:-${WEB_OUTPUT_DIR:-$ROOT/generated/raspberry-pi}}"

latest_for() {
  local carrier="$1" found=""
  [[ -d "$RESULTS_ROOT" ]] || return 1
  found="$(find "$RESULTS_ROOT" -mindepth 1 -maxdepth 1 -type d -name "*-${carrier}" -printf '%T@ %p\n' 2>/dev/null | sort -nr | head -1 | cut -d' ' -f2-)"
  [[ -n "$found" ]] && printf '%s\n' "$found"
}

if [[ -z "$TCP_INPUT" ]]; then TCP_INPUT="$(latest_for tcp || true)"; fi
if [[ -z "$UDP_INPUT" ]]; then UDP_INPUT="$(latest_for udp || true)"; fi
[[ -n "$TCP_INPUT" && -d "$TCP_INPUT" ]] || { echo "[FAIL] TCP results not found. Pass TCP_RESULT_DIR or set TCP_RESULTS_DIR." >&2; exit 1; }
[[ -n "$UDP_INPUT" && -d "$UDP_INPUT" ]] || { echo "[FAIL] UDP results not found. Pass UDP_RESULT_DIR or set UDP_RESULTS_DIR." >&2; exit 1; }

rm -rf "$OUTPUT"
mkdir -p "$OUTPUT"
COMMON=(--platform "${PLATFORM_NAME:-Raspberry Pi}" --commit "${GIT_COMMIT:-$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo unknown)}" --kernel "${BENCHMARK_KERNEL:-unknown}" --compiler "${BENCHMARK_COMPILER:-unknown}")
python3 "$ROOT/generate_dashboard.py" "$TCP_INPUT" "$OUTPUT/tcp" --transport tcp --title "${TCP_PAGE_TITLE:-Raspberry Pi / TCP carrier benchmark}" "${COMMON[@]}"
python3 "$ROOT/generate_dashboard.py" "$UDP_INPUT" "$OUTPUT/udp" --transport udp --title "${UDP_PAGE_TITLE:-Raspberry Pi / UDP carrier benchmark}" "${COMMON[@]}"
python3 "$ROOT/generate_site.py" "$OUTPUT" --tcp-page "$OUTPUT/tcp" --udp-page "$OUTPUT/udp" --platform "${PLATFORM_NAME:-Raspberry Pi}"

# Rebuild root manifest after child dashboards are complete.
python3 - "$OUTPUT" <<'PY'
import datetime as dt, hashlib, json, sys
from pathlib import Path
root=Path(sys.argv[1]); files=[]
for p in sorted(root.rglob('*')):
    if p.is_file() and p.name!='manifest.json':
        files.append({'path':str(p.relative_to(root)),'bytes':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()})
(root/'manifest.json').write_text(json.dumps({'generated_at':dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat(),'files':files},indent=2)+'\n')
print(f'[ OK ] Combined manifest: {len(files)} files')
PY

if [[ "${AUTO_PUBLISH_WEB:-0}" == "1" ]]; then "$ROOT/publish-all.sh" "$OUTPUT"; fi
printf '[ OK ] TCP + UDP website generated: %s\n' "$OUTPUT"
