#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
RESULT_DIR="${1:?Usage: $0 RESULT_DIR [tcp|udp|both]}"
MODE="${2:-both}"

GENERATOR="${GENERATOR:-$SCRIPT_DIR/generate-benchmark-site.py}"
REMOTE_HOST="${REMOTE_HOST:-www-data@fuji}"
REMOTE_BASE="${REMOTE_BASE:-/var/www/html/public/stcp.fi/benchmarks/raspberry-pi}"
OUTPUT_DIR="${OUTPUT_DIR:-/tmp/stcp-raspberry-site-$$}"
KEEP_OUTPUT="${KEEP_OUTPUT:-0}"
DRY_RUN="${DRY_RUN:-0}"

log(){ printf '[INFO] %s\n' "$*"; }
ok(){ printf '[ OK ] %s\n' "$*"; }
die(){ printf '[FAIL] %s\n' "$*" >&2; exit 1; }

case "$MODE" in tcp|udp|both) ;; *) die "Unknown mode: $MODE";; esac
command -v python3 >/dev/null || die "python3 is required"
command -v rsync >/dev/null || die "rsync is required"
command -v ssh >/dev/null || die "ssh is required"
[[ -f "$GENERATOR" ]] || die "Generator missing: $GENERATOR"
RESULT_DIR="$(cd -- "$RESULT_DIR" && pwd -P)" || die "Result directory not found"

cleanup(){ [[ "$KEEP_OUTPUT" == 1 ]] || rm -rf -- "$OUTPUT_DIR"; }
trap cleanup EXIT
rm -rf -- "$OUTPUT_DIR"
mkdir -p -- "$OUTPUT_DIR"

log "Generating site from: $RESULT_DIR"
python3 "$GENERATOR" --mode "$MODE" --result-dir "$RESULT_DIR" --output-dir "$OUTPUT_DIR"

modes=("$MODE")
[[ "$MODE" == both ]] && modes=(tcp udp)
for carrier in "${modes[@]}"; do
  for required in index.html pipeline-summary.json dashboard-data.json summary.json cases.csv report.md manifest.json; do
    [[ -s "$OUTPUT_DIR/$carrier/$required" ]] || die "Generated file missing: $carrier/$required"
  done
  python3 -m json.tool "$OUTPUT_DIR/$carrier/pipeline-summary.json" >/dev/null
  python3 -m json.tool "$OUTPUT_DIR/$carrier/dashboard-data.json" >/dev/null
  python3 -m json.tool "$OUTPUT_DIR/$carrier/summary.json" >/dev/null
  python3 -m json.tool "$OUTPUT_DIR/$carrier/manifest.json" >/dev/null
  ok "Validated generated $carrier site"
done

if [[ "$DRY_RUN" == 1 ]]; then
  ok "Dry run complete: $OUTPUT_DIR"
  KEEP_OUTPUT=1
  exit 0
fi

log "Creating remote target: $REMOTE_HOST:$REMOTE_BASE"
ssh "$REMOTE_HOST" "mkdir -p '$REMOTE_BASE'"

# Publish each carrier separately so tcp-only does not delete udp and vice versa.
for carrier in "${modes[@]}"; do
  remote_stage="$REMOTE_BASE/.${carrier}.new.$$"
  remote_live="$REMOTE_BASE/$carrier"
  remote_old="$REMOTE_BASE/.${carrier}.old.$$"

  log "Uploading $carrier to staging directory"
  ssh "$REMOTE_HOST" "rm -rf '$remote_stage' && mkdir -p '$remote_stage'"
  rsync -a --delete "$OUTPUT_DIR/$carrier/" "$REMOTE_HOST:$remote_stage/"

  log "Activating $carrier atomically"
  ssh "$REMOTE_HOST" "set -e; rm -rf '$remote_old'; if [ -e '$remote_live' ]; then mv '$remote_live' '$remote_old'; fi; if mv '$remote_stage' '$remote_live'; then rm -rf '$remote_old'; else [ ! -e '$remote_old' ] || mv '$remote_old' '$remote_live'; exit 1; fi"
  ok "Published $carrier"
done

# Root index is safe to refresh after selected carrier directories are active.
rsync -a "$OUTPUT_DIR/index.html" "$REMOTE_HOST:$REMOTE_BASE/index.html"
ok "stcp.fi Raspberry Pi benchmark site updated"
