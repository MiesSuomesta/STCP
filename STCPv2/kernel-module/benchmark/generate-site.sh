#!/usr/bin/env bash
set -Eeuo pipefail

# Generate a complete site into a new /tmp/www-$$ directory.
# This script never modifies the live website.
#
# Usage:
#   generate-site.sh RESULT_DIR [tcp|udp|both]
#
# Configure the existing project-specific generator with either:
#   SITE_GENERATOR=/path/to/generator
# or:
#   SITE_GENERATOR_CMD='python3 ... --results "$RESULT_DIR" --output "$OUTPUT_DIR"'
#
# If no generator is configured, common project filenames are auto-detected.

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd -P)"

RESULT_DIR="${1:?Usage: $0 RESULT_DIR [tcp|udp|both]}"
MODE="${2:-both}"
OUTPUT_DIR="${OUTPUT_DIR:-/tmp/www-$$}"
SITE_GENERATOR="${SITE_GENERATOR:-}"
SITE_GENERATOR_CMD="${SITE_GENERATOR_CMD:-}"

log()  { printf '[INFO] %s\n' "$*"; }
ok()   { printf '[ OK ] %s\n' "$*"; }
die()  { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

case "$MODE" in tcp|udp|both) ;; *) die "Unknown mode: $MODE" ;; esac

RESULT_DIR="$(cd -- "$RESULT_DIR" 2>/dev/null && pwd -P)" ||
    die "Result directory not found: $RESULT_DIR"

rm -rf -- "$OUTPUT_DIR"
mkdir -p "$OUTPUT_DIR"
OUTPUT_DIR="$(cd -- "$OUTPUT_DIR" && pwd -P)"

# Static site base, when present.
for static_dir in "$ROOT/web" "$ROOT/site" "$ROOT/www"; do
    if [[ -d "$static_dir" ]]; then
        log "Copying static files from $static_dir"
        cp -a "$static_dir/." "$OUTPUT_DIR/"
        break
    fi
done

if [[ -n "$SITE_GENERATOR_CMD" ]]; then
    log "Running configured SITE_GENERATOR_CMD"
    export RESULT_DIR OUTPUT_DIR MODE ROOT SCRIPT_DIR
    bash -c "$SITE_GENERATOR_CMD"
else
    if [[ -z "$SITE_GENERATOR" ]]; then
        candidates=(
            "$ROOT/generate-benchmark-site.py"
            "$ROOT/generate-benchmark-site.sh"
            "$SCRIPT_DIR/generate-benchmark-site.py"
            "$SCRIPT_DIR/generate-benchmark-site.sh"
            "$ROOT/publish-benchmark-site.sh"
        )

        for candidate in "${candidates[@]}"; do
            if [[ -f "$candidate" ]]; then
                SITE_GENERATOR="$candidate"
                break
            fi
        done
    fi

    [[ -n "$SITE_GENERATOR" && -f "$SITE_GENERATOR" ]] ||
        die "No site generator found. Set SITE_GENERATOR or SITE_GENERATOR_CMD."

    log "Generator: $SITE_GENERATOR"

    case "$SITE_GENERATOR" in
        *.py)
            python3 "$SITE_GENERATOR" \
                --mode "$MODE" \
                --result-dir "$RESULT_DIR" \
                --output-dir "$OUTPUT_DIR"
            ;;
        *.sh)
            # Contract for the new pipeline:
            #   generator MODE RESULT_DIR OUTPUT_DIR
            bash "$SITE_GENERATOR" "$MODE" "$RESULT_DIR" "$OUTPUT_DIR"
            ;;
        *)
            die "Unsupported generator type: $SITE_GENERATOR"
            ;;
    esac
fi

find "$OUTPUT_DIR" -mindepth 1 -print -quit | grep -q . ||
    die "Generator produced an empty directory: $OUTPUT_DIR"

printf '%s\n' "$RESULT_DIR" >"$OUTPUT_DIR/.benchmark-result-source"
printf '%s\n' "$(date --iso-8601=seconds)" >"$OUTPUT_DIR/.benchmark-generated-at"

ok "Generated website: $OUTPUT_DIR"
printf '%s\n' "$OUTPUT_DIR"
