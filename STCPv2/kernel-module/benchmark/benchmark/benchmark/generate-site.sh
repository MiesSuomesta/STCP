#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd -P)"

RESULT_DIR="${1:?Usage: $0 RESULT_DIR [tcp|udp|both]}"
MODE="${2:-both}"
OUTPUT_DIR="${OUTPUT_DIR:-/tmp/www-$$}"

SITE_GENERATOR="${SITE_GENERATOR:-}"
SITE_GENERATOR_CMD="${SITE_GENERATOR_CMD:-}"
SITE_INDEX="${SITE_INDEX:-index.html}"
STATIC_SITE_DIR="${STATIC_SITE_DIR:-}"

SUMMARY_GENERATOR="${SUMMARY_GENERATOR:-$SCRIPT_DIR/generate-summary-from-results.py}"
SUMMARY_INJECTOR="${SUMMARY_INJECTOR:-$SCRIPT_DIR/inject-summary-into-site.py}"

log()  { printf '[INFO] %s\n' "$*"; }
ok()   { printf '[ OK ] %s\n' "$*"; }
die()  { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

case "$MODE" in tcp|udp|both) ;; *) die "Unknown mode: $MODE" ;; esac

RESULT_DIR="$(cd -- "$RESULT_DIR" 2>/dev/null && pwd -P)" ||
    die "Result directory not found: $RESULT_DIR"

[[ -f "$SUMMARY_GENERATOR" ]] || die "Missing summary generator: $SUMMARY_GENERATOR"
[[ -f "$SUMMARY_INJECTOR" ]] || die "Missing summary injector: $SUMMARY_INJECTOR"

rm -rf -- "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"
OUTPUT_DIR="$(cd -- "$OUTPUT_DIR" && pwd -P)"

log "Result set: $RESULT_DIR"
log "Output:     $OUTPUT_DIR"

if [[ ! -f "$RESULT_DIR/pipeline-summary.json" ]]; then
    log "Generating pipeline-summary.json from existing results"
    python3 "$SUMMARY_GENERATOR" "$RESULT_DIR" --mode "$MODE"
else
    log "Using existing pipeline-summary.json"
fi

if [[ -n "$STATIC_SITE_DIR" ]]; then
    [[ -d "$STATIC_SITE_DIR" ]] || die "STATIC_SITE_DIR does not exist: $STATIC_SITE_DIR"
    cp -a "$STATIC_SITE_DIR/." "$OUTPUT_DIR/"
else
    for static_dir in "$ROOT/web" "$ROOT/site" "$ROOT/www"; do
        if [[ -d "$static_dir" ]]; then
            log "Copying static site from $static_dir"
            cp -a "$static_dir/." "$OUTPUT_DIR/"
            break
        fi
    done
fi

if [[ -n "$SITE_GENERATOR_CMD" ]]; then
    log "Running SITE_GENERATOR_CMD"
    export RESULT_DIR OUTPUT_DIR MODE ROOT SCRIPT_DIR
    bash -c "$SITE_GENERATOR_CMD"
else
    if [[ -z "$SITE_GENERATOR" ]]; then
        candidates=(
            "$ROOT/generate-benchmark-site.py"
            "$ROOT/generate-benchmark-site.sh"
            "$SCRIPT_DIR/generate-benchmark-site.py"
            "$SCRIPT_DIR/generate-benchmark-site.sh"
        )
        for candidate in "${candidates[@]}"; do
            if [[ -f "$candidate" ]]; then
                SITE_GENERATOR="$candidate"
                break
            fi
        done
    fi

    if [[ -n "$SITE_GENERATOR" ]]; then
        log "Running site generator: $SITE_GENERATOR"
        case "$SITE_GENERATOR" in
            *.py)
                python3 "$SITE_GENERATOR" --mode "$MODE" --result-dir "$RESULT_DIR" --output-dir "$OUTPUT_DIR"
                ;;
            *.sh)
                bash "$SITE_GENERATOR" "$MODE" "$RESULT_DIR" "$OUTPUT_DIR"
                ;;
            *)
                die "Unsupported generator type: $SITE_GENERATOR"
                ;;
        esac
    else
        log "No separate generator configured; using copied static site"
    fi
fi

[[ -f "$OUTPUT_DIR/$SITE_INDEX" ]] || die "Generated landing page missing: $OUTPUT_DIR/$SITE_INDEX"

log "Installing benchmark summary into generated site"
python3 "$SUMMARY_INJECTOR"     --result-dir "$RESULT_DIR"     --site-dir "$OUTPUT_DIR"     --index "$SITE_INDEX"

printf '%s\n' "$RESULT_DIR" >"$OUTPUT_DIR/.benchmark-result-source"
printf '%s\n' "$(date --iso-8601=seconds)" >"$OUTPUT_DIR/.benchmark-generated-at"

ok "Generated website: $OUTPUT_DIR"
printf '%s\n' "$OUTPUT_DIR"
