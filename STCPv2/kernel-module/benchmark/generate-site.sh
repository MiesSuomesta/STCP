#!/usr/bin/env bash
set -Eeuo pipefail

# Generate an stcp.fi benchmark site from an explicit result directory.
#
# This script:
#   - never searches outside the current kernel-module project tree
#   - never uses *-oli, old, backup or temporary source directories
#   - never invokes publish-benchmark-site.sh as a generator
#   - generates pipeline-summary.json from existing results when needed
#   - copies current web/ static files
#   - copies benchmark JSON data into the generated site
#   - injects the summary card into index.html
#
# Usage:
#   generate-site.sh RESULT_DIR [tcp|udp|both]
#
# Environment:
#   OUTPUT_DIR=/tmp/www-$$
#   SITE_GENERATOR=/exact/path/to/current/generator.py
#   SITE_GENERATOR_CMD='...'
#   STATIC_SITE_DIR=/exact/path/to/current/web
#   SITE_INDEX=index.html

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd -P)"

RESULT_DIR="${1:?Usage: $0 RESULT_DIR [tcp|udp|both]}"
MODE="${2:-both}"

OUTPUT_DIR="${OUTPUT_DIR:-/tmp/www-$$}"
SITE_INDEX="${SITE_INDEX:-index.html}"

SITE_GENERATOR="${SITE_GENERATOR:-}"
SITE_GENERATOR_CMD="${SITE_GENERATOR_CMD:-}"
STATIC_SITE_DIR="${STATIC_SITE_DIR:-$ROOT/web}"

SUMMARY_GENERATOR="${SUMMARY_GENERATOR:-$SCRIPT_DIR/generate-summary-from-results.py}"
SUMMARY_INJECTOR="${SUMMARY_INJECTOR:-$SCRIPT_DIR/inject-summary-into-site.py}"

log()  { printf '[INFO] %s\n' "$*"; }
ok()   { printf '[ OK ] %s\n' "$*"; }
warn() { printf '[WARN] %s\n' "$*" >&2; }
die()  { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

case "$MODE" in
    tcp|udp|both) ;;
    *) die "Unknown mode: $MODE" ;;
esac

RESULT_DIR="$(cd -- "$RESULT_DIR" 2>/dev/null && pwd -P)" ||
    die "Result directory not found: $RESULT_DIR"

case "$RESULT_DIR" in
    *-oli*|*"/old/"*|*"/backup/"*|*"/tmp/"*)
        die "Refusing temporary/old result source: $RESULT_DIR"
        ;;
esac

[[ -f "$SUMMARY_GENERATOR" ]] ||
    die "Missing summary generator: $SUMMARY_GENERATOR"

[[ -f "$SUMMARY_INJECTOR" ]] ||
    die "Missing summary injector: $SUMMARY_INJECTOR"

if [[ ! -d "$STATIC_SITE_DIR" ]]; then
    if [[ -f "$SCRIPT_DIR/generate-benchmark-site.py" ]]; then
        warn "Static web directory missing; standalone benchmark generator will build the site"
        STATIC_SITE_DIR=""
    else
        die "Current static web directory missing: $STATIC_SITE_DIR"
    fi
fi

rm -rf -- "$OUTPUT_DIR"
mkdir -p -- "$OUTPUT_DIR"
OUTPUT_DIR="$(cd -- "$OUTPUT_DIR" && pwd -P)"

log "Mode:       $MODE"
log "Result set: $RESULT_DIR"
log "Web source: ${STATIC_SITE_DIR:-standalone generator}"
log "Output:     $OUTPUT_DIR"

# Always refresh the summary from the actual selected result directory.
log "Generating pipeline-summary.json"
python3 "$SUMMARY_GENERATOR" \
    "$RESULT_DIR" \
    --mode "$MODE"

if [[ -n "$STATIC_SITE_DIR" ]]; then
    log "Copying current static website"
    cp -a -- "$STATIC_SITE_DIR/." "$OUTPUT_DIR/"
fi

# Optional current-project generator. It is used only when explicitly supplied
# or when one of the safe generator filenames exists inside the current tree.
if [[ -n "$SITE_GENERATOR_CMD" ]]; then
    log "Running configured SITE_GENERATOR_CMD"

    export RESULT_DIR OUTPUT_DIR MODE ROOT SCRIPT_DIR
    bash -c "$SITE_GENERATOR_CMD"
else
    if [[ -n "$SITE_GENERATOR" ]]; then
        SITE_GENERATOR="$(realpath -e "$SITE_GENERATOR")"

        case "$SITE_GENERATOR" in
            "$ROOT"/*) ;;
            *) die "SITE_GENERATOR must be inside the current project tree: $ROOT" ;;
        esac
    else
        candidates=(
            "$SCRIPT_DIR/generate-benchmark-site.py"
            "$SCRIPT_DIR/generate-benchmark-site.sh"
            "$ROOT/generate-benchmark-site.py"
            "$ROOT/generate-benchmark-site.sh"
            "$ROOT/web/generate-benchmark-site.py"
            "$ROOT/web/generate-benchmark-site.sh"
        )

        for candidate in "${candidates[@]}"; do
            [[ -f "$candidate" ]] || continue
            SITE_GENERATOR="$candidate"
            break
        done
    fi

    if [[ -n "$SITE_GENERATOR" ]]; then
        log "Running current-project generator: $SITE_GENERATOR"

        case "$SITE_GENERATOR" in
            *publish*.sh|*publish*.py)
                die "Publish scripts cannot be used as site generators: $SITE_GENERATOR"
                ;;
            *.py)
                python3 "$SITE_GENERATOR" \
                    --mode "$MODE" \
                    --result-dir "$RESULT_DIR" \
                    --output-dir "$OUTPUT_DIR"
                ;;
            *.sh)
                bash "$SITE_GENERATOR" \
                    "$MODE" \
                    "$RESULT_DIR" \
                    "$OUTPUT_DIR"
                ;;
            *)
                die "Unsupported generator type: $SITE_GENERATOR"
                ;;
        esac
    else
        warn "No separate benchmark generator found; using current web/ files and raw benchmark data"
    fi
fi

# Publish the selected source data inside the generated site. This makes the
# generated directory self-contained and lets JavaScript load the result JSONs.
DATA_DIR="$OUTPUT_DIR/benchmark-data"
mkdir -p -- "$DATA_DIR"

for carrier in tcp udp; do
    if [[ -d "$RESULT_DIR/$carrier" ]]; then
        mkdir -p -- "$DATA_DIR/$carrier"

        find "$RESULT_DIR/$carrier" \
            -maxdepth 1 \
            -type f \
            \( -name '*.json' -o -name '*.csv' \) \
            -exec cp -a -- {} "$DATA_DIR/$carrier/" \;
    fi
done

for metadata in \
    pipeline-summary.json \
    run-summary.tsv \
    cases.tsv \
    case-timings.tsv
do
    if [[ -f "$RESULT_DIR/$metadata" ]]; then
        cp -a -- "$RESULT_DIR/$metadata" "$DATA_DIR/$metadata"
    fi
done

# Resolve the landing page. Prefer the configured path, then common current
# project layouts. If none exists, generate a minimal benchmark landing page.
if [[ ! -f "$OUTPUT_DIR/$SITE_INDEX" ]]; then
    landing_candidates=(
        "index.html"
        "public/index.html"
        "html/index.html"
        "site/index.html"
        "www/index.html"
        "benchmarks/index.html"
        "benchmark/index.html"
    )

    resolved_index=""

    for candidate in "${landing_candidates[@]}"; do
        if [[ -f "$OUTPUT_DIR/$candidate" ]]; then
            resolved_index="$candidate"
            break
        fi
    done

    if [[ -z "$resolved_index" ]]; then
        resolved_index="$(
            find "$OUTPUT_DIR" \
                -mindepth 1 -maxdepth 4 \
                -type f -name index.html \
                -printf '%P\n' |
            sort |
            head -1
        )"
    fi

    if [[ -n "$resolved_index" ]]; then
        SITE_INDEX="$resolved_index"
        log "Detected landing page: $SITE_INDEX"
    else
        SITE_INDEX="index.html"
        warn "No landing page found; generating benchmark index.html"

        cat >"$OUTPUT_DIR/$SITE_INDEX" <<'HTML'
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width,initial-scale=1">
  <title>STCP benchmark results</title>
  <style>
    :root { color-scheme: dark; }
    body {
      margin: 0;
      font-family: system-ui, sans-serif;
      background: #020617;
      color: #e2e8f0;
    }
    main {
      width: min(1100px, calc(100% - 2rem));
      margin: 0 auto;
      padding: 2rem 0 4rem;
    }
    h1 { margin-bottom: .4rem; }
    p { color: #94a3b8; }
    a { color: #7dd3fc; }
    .links {
      display: flex;
      flex-wrap: wrap;
      gap: .8rem;
      margin: 1.5rem 0;
    }
    .links a {
      padding: .65rem .9rem;
      border: 1px solid #334155;
      border-radius: 10px;
      text-decoration: none;
      background: #0f172a;
    }
  </style>
</head>
<body>
  <main>
    <h1>STCP benchmark results</h1>
    <p>Generated from the selected benchmark result set.</p>
    <nav class="links">
      <a href="./pipeline-summary.json">Pipeline summary JSON</a>
      <a href="./benchmark-data/">Benchmark data</a>
    </nav>
  </main>
</body>
</html>
HTML
    fi
fi

log "Installing benchmark summary card into $SITE_INDEX"
python3 "$SUMMARY_INJECTOR" \
    --result-dir "$RESULT_DIR" \
    --site-dir "$OUTPUT_DIR" \
    --index "$SITE_INDEX"

printf '%s\n' "$RESULT_DIR" >"$OUTPUT_DIR/.benchmark-result-source"
printf '%s\n' "$(date --iso-8601=seconds)" >"$OUTPUT_DIR/.benchmark-generated-at"

ok "Generated website: $OUTPUT_DIR"
ok "Benchmark data:    $DATA_DIR"
printf '%s\n' "$OUTPUT_DIR"
