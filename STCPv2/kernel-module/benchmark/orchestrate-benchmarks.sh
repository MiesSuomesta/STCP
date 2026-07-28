#!/usr/bin/env bash
set -Eeuo pipefail

# Orchestrate the complete benchmark pipeline:
#   1. run the whole matrix
#   2. generate a website into /tmp/www-$$
#   3. back up the current live site
#   4. publish the generated site atomically
#
# Usage:
#   orchestrate-benchmarks.sh tcp|udp|both LIVE_DIR
#
# Example:
#   sudo -E bash benchmark/orchestrate-benchmarks.sh both /var/www/stcp.fi
#
# Important environment variables:
#   CASE_FILE=benchmark/cases.tsv
#   RESULT_DIR=benchmark/results/full-YYYYmmdd-HHMMSS
#   SITE_GENERATOR=/path/to/generator
#   SITE_GENERATOR_CMD='...'
#   BACKUP_ROOT=/var/backups/stcp-benchmark
#   KEEP_BACKUPS=10
#   SKIP_BENCHMARKS=0
#   SKIP_GENERATE=0
#   SKIP_DEPLOY=0
#   KEEP_GENERATED=0
#   AUTO_RESUME=1
#   FORCE_RERUN=0
#   RESUME_LATEST=0
#   RESUME_MIN_DURATION_RATIO=0.90
#   RESUME_NOTIFY_SKIPS=0

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd -P)"

MODE="${1:-}"
LIVE_DIR="${2:-}"

RUN_ALL="${RUN_ALL:-$SCRIPT_DIR/run-all.sh}"
GENERATE_SITE="${GENERATE_SITE:-$SCRIPT_DIR/generate-site.sh}"
DEPLOY_SITE="${DEPLOY_SITE:-$SCRIPT_DIR/deploy-site.sh}"

RESULT_DIR_WAS_SET=0
[[ -n "${RESULT_DIR+x}" ]] && RESULT_DIR_WAS_SET=1

RESULT_DIR="${RESULT_DIR:-$SCRIPT_DIR/results/full-$(date +%Y%m%d-%H%M%S)}"
GENERATED_DIR="${GENERATED_DIR:-/tmp/www-$$}"

SKIP_BENCHMARKS="${SKIP_BENCHMARKS:-0}"
SKIP_GENERATE="${SKIP_GENERATE:-0}"
SKIP_DEPLOY="${SKIP_DEPLOY:-0}"
KEEP_GENERATED="${KEEP_GENERATED:-0}"

# Resume controls passed to run-all.sh.
AUTO_RESUME="${AUTO_RESUME:-1}"
FORCE_RERUN="${FORCE_RERUN:-0}"
RESUME_LATEST="${RESUME_LATEST:-0}"
RESUME_MIN_DURATION_RATIO="${RESUME_MIN_DURATION_RATIO:-0.90}"
RESUME_NOTIFY_SKIPS="${RESUME_NOTIFY_SKIPS:-0}"

LOCK_FILE="${LOCK_FILE:-/tmp/stcp-benchmark-pipeline.lock}"
PIPELINE_LOG_DIR="${PIPELINE_LOG_DIR:-$SCRIPT_DIR/pipeline-logs}"
STAMP="$(date +%Y%m%d-%H%M%S)"
PIPELINE_LOG="${PIPELINE_LOG:-$PIPELINE_LOG_DIR/orchestrate-$STAMP.log}"

log()  { printf '[INFO] %s\n' "$*"; }
ok()   { printf '[ OK ] %s\n' "$*"; }
warn() { printf '[WARN] %s\n' "$*" >&2; }
die()  { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

usage() {
    cat <<EOF
Usage:
  $0 tcp|udp|both LIVE_DIR

Examples:
  $0 both /var/www/stcp.fi

  CASE_FILE=benchmark/cases.tsv \\
  SITE_GENERATOR=/path/to/generator.py \\
  sudo -E $0 both /var/www/stcp.fi

Resume from an existing result set:
  SKIP_BENCHMARKS=1 \\
  RESULT_DIR=benchmark/results/full-20260727-190000 \\
  sudo -E $0 both /var/www/stcp.fi
EOF
}

find_latest_result_dir() {
    local latest

    latest="$(
        find "$SCRIPT_DIR/results" \
            -mindepth 1 -maxdepth 1 \
            -type d \
            -name 'full-*' \
            -printf '%T@ %p\n' 2>/dev/null |
        sort -nr |
        awk 'NR == 1 {sub(/^[^ ]+ /, ""); print; exit}'
    )"

    [[ -n "$latest" ]] || return 1
    printf '%s\n' "$latest"
}

[[ -n "$MODE" && -n "$LIVE_DIR" ]] || {
    usage >&2
    exit 2
}

case "$MODE" in
    tcp|udp|both) ;;
    *) die "Unknown mode: $MODE" ;;
esac

for script in "$RUN_ALL" "$GENERATE_SITE" "$DEPLOY_SITE"; do
    [[ -f "$script" ]] || die "Required script missing: $script"
done

command -v flock >/dev/null || die "flock is required"

mkdir -p "$PIPELINE_LOG_DIR"
exec > >(tee -a "$PIPELINE_LOG") 2>&1

if [[ "$RESUME_LATEST" == 1 && "$RESULT_DIR_WAS_SET" != 1 ]]; then
    latest_result_dir="$(find_latest_result_dir || true)"

    if [[ -n "$latest_result_dir" ]]; then
        RESULT_DIR="$latest_result_dir"
        log "Auto-selected latest result set for resume: $RESULT_DIR"
    else
        log "No previous result set found; a new result set will be created"
    fi
fi

# Prevent two full benchmark/publish pipelines from running simultaneously.
exec 9>"$LOCK_FILE"
if ! flock -n 9; then
    die "Another benchmark pipeline is already running: $LOCK_FILE"
fi

cleanup() {
    rc=$?

    if (( rc != 0 )); then
        warn "Pipeline failed with exit code $rc"
        warn "Result directory was preserved: $RESULT_DIR"
        [[ -d "$GENERATED_DIR" ]] &&
            warn "Generated directory was preserved: $GENERATED_DIR"
    elif [[ "$KEEP_GENERATED" != 1 && -d "$GENERATED_DIR" ]]; then
        rm -rf -- "$GENERATED_DIR"
        log "Removed temporary generated directory"
    fi

    exit "$rc"
}
trap cleanup EXIT

RESULT_DIR="$(mkdir -p -- "$RESULT_DIR" && cd -- "$RESULT_DIR" && pwd -P)"
LIVE_PARENT="$(dirname -- "$LIVE_DIR")"
mkdir -p "$LIVE_PARENT"
LIVE_DIR="$(cd -- "$LIVE_PARENT" && pwd -P)/$(basename -- "$LIVE_DIR")"

cat <<EOF
STCP benchmark pipeline
-----------------------
Mode:            $MODE
Result set:      $RESULT_DIR
Generated site:  $GENERATED_DIR
Live site:       $LIVE_DIR
Pipeline log:    $PIPELINE_LOG
Auto resume:     $AUTO_RESUME
Force rerun:     $FORCE_RERUN
Resume latest:   $RESUME_LATEST
EOF

if [[ "$SKIP_BENCHMARKS" != 1 ]]; then
    if [[ "$AUTO_RESUME" == 1 ]] &&
       find "$RESULT_DIR" -mindepth 2 -maxdepth 2 -type f -name '*.json' -print -quit |
       grep -q .
    then
        log "Stage 1/3: resuming benchmark matrix from existing results"
    else
        log "Stage 1/3: running benchmark matrix"
    fi

    AUTO_RESUME="$AUTO_RESUME"     FORCE_RERUN="$FORCE_RERUN"     RESUME_MIN_DURATION_RATIO="$RESUME_MIN_DURATION_RATIO"     RESUME_NOTIFY_SKIPS="$RESUME_NOTIFY_SKIPS"         bash "$RUN_ALL" "$MODE" "$RESULT_DIR"

    ok "Benchmark matrix completed"
else
    warn "Stage 1/3 skipped: using existing result set"
    [[ -d "$RESULT_DIR" ]] || die "Existing RESULT_DIR does not exist"
fi

if [[ "$SKIP_GENERATE" != 1 ]]; then
    log "Stage 2/3: generating website"
    OUTPUT_DIR="$GENERATED_DIR" \
        bash "$GENERATE_SITE" "$RESULT_DIR" "$MODE"
    ok "Website generation completed"
else
    warn "Stage 2/3 skipped: using existing generated directory"
    [[ -d "$GENERATED_DIR" ]] || die "Existing GENERATED_DIR does not exist"
fi

if [[ "$SKIP_DEPLOY" != 1 ]]; then
    log "Stage 3/3: backing up and publishing website"
    bash "$DEPLOY_SITE" "$GENERATED_DIR" "$LIVE_DIR"
    ok "Website publication completed"
else
    warn "Stage 3/3 skipped: site was not published"
fi

cat <<EOF

Pipeline completed
------------------
Mode:       $MODE
Results:    $RESULT_DIR
Live site:  $LIVE_DIR
Log:        $PIPELINE_LOG
EOF
