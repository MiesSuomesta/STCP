#!/usr/bin/env bash
#
# run-benchmark-orchestrator.sh
#
# Complete STCP Raspberry benchmark orchestration:
#   - preflight checks
#   - optional build and module deployment
#   - TCP, UDP or both carrier matrices
#   - result validation
#   - result archive
#   - static dashboard generation
#   - automatic publication to stcp.fi
#   - optional golden commit/tag/push after a completely successful run
#
# Usage:
#   ./run-benchmark-orchestrator.sh both
#   ./run-benchmark-orchestrator.sh tcp
#   ./run-benchmark-orchestrator.sh udp
#   ./run-benchmark-orchestrator.sh publish /path/to/full-result
#
# Common examples:
#
#   CARRIERS=tcp \
#   CLIENTS_LIST="1 2 4 8 16" \
#   PAYLOADS="64 1024 4096 65536 262144 1048576" \
#   PIPELINES="1 4 8" \
#   ./run-benchmark-orchestrator.sh tcp
#
#   AUTO_GOLDEN=1 AUTO_PUSH=1 ./run-benchmark-orchestrator.sh both
#

set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT="${ROOT:-$SCRIPT_DIR}"
MODE="${1:-both}"
PUBLISH_RESULT_DIR="${2:-}"

PIPELINE="${PIPELINE:-$ROOT/build-benchmark-publish.sh}"
PUBLISH_TOOL="${PUBLISH_TOOL:-$ROOT/publish-latest-benchmarks.sh}"
RESULTS_ROOT="${RESULTS_ROOT:-$ROOT/benchmark/results}"
LOG_DIR="${LOG_DIR:-$ROOT/benchmark/pipeline-logs}"

RPI_ADDR="${RPI_ADDR:-192.168.1.199}"
RPI_USER="${RPI_USER:-pi}"
RPI_BENCHMARK_DIR="${RPI_BENCHMARK_DIR:-/home/pi/benchmark}"
WEB_DEPLOY_TARGET="${WEB_DEPLOY_TARGET:-www-data@fuji:/var/www/html/public/stcp.fi/benchmarks/raspberry-pi/}"

DURATION="${DURATION:-15}"
CLIENTS_LIST="${CLIENTS_LIST:-16 8 4 2 1}"
PAYLOADS="${PAYLOADS:-1048576 262144 131072 65536 4096 1024 64}"
PIPELINES="${PIPELINES:-8 4 1}"

VERIFY="${VERIFY:-0}"
IRQ_METRICS="${IRQ_METRICS:-1}"
PERF_METRICS="${PERF_METRICS:-1}"
CONTINUE_ON_ERROR="${CONTINUE_ON_ERROR:-1}"
SYNC_RPI="${SYNC_RPI:-1}"
CARRIER_DEBUG="${CARRIER_DEBUG:-0}"
CLEAN_OLD_RESULTS="${CLEAN_OLD_RESULTS:-0}"
KEEP_RESULT_RUNS="${KEEP_RESULT_RUNS:-5}"

BUILD_FIRST="${BUILD_FIRST:-0}"
AUTO_PUBLISH_WEB="${AUTO_PUBLISH_WEB:-1}"
AUTO_ARCHIVE="${AUTO_ARCHIVE:-1}"

AUTO_GOLDEN="${AUTO_GOLDEN:-0}"
AUTO_PUSH="${AUTO_PUSH:-0}"
GOLDEN_TAG_PREFIX="${GOLDEN_TAG_PREFIX:-stcp-golden}"
GIT_REMOTE="${GIT_REMOTE:-origin}"

STAMP="${STAMP:-$(date +%Y%m%d-%H%M%S)}"
LOG_FILE="${LOG_FILE:-$LOG_DIR/orchestrator-$STAMP.log}"

CURRENT_PHASE="startup"
START_SECONDS="$SECONDS"
LAST_RESULT_DIR=""
RESULT_ARCHIVE=""
SELECTED_CARRIERS=""

mkdir -p "$LOG_DIR" "$RESULTS_ROOT"
exec > >(tee -a "$LOG_FILE") 2>&1

log()  { printf '[INFO] %s\n' "$*"; }
ok()   { printf '[ OK ] %s\n' "$*"; }
warn() { printf '[WARN] %s\n' "$*" >&2; }
die()  { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

need_cmd() {
    command -v "$1" >/dev/null 2>&1 || die "Missing command: $1"
}

on_error() {
    local rc=$?
    printf '\n[FAIL] Orchestration failed in phase: %s (exit=%d)\n' \
        "$CURRENT_PHASE" "$rc" >&2
    printf '[INFO] Log: %s\n' "$LOG_FILE" >&2
    exit "$rc"
}

on_exit() {
    local rc=$?
    local elapsed=$((SECONDS - START_SECONDS))

    printf '\n== STCP benchmark orchestration summary ==\n'
    printf 'Mode:        %s\n' "$MODE"
    printf 'Carriers:    %s\n' "$SELECTED_CARRIERS"
    printf 'Result:      %s\n' "${LAST_RESULT_DIR:-not created}"
    printf 'Archive:     %s\n' "${RESULT_ARCHIVE:-not created}"
    printf 'Published:   %s\n' "$AUTO_PUBLISH_WEB"
    printf 'Elapsed:     %dm %02ds\n' "$((elapsed / 60))" "$((elapsed % 60))"
    printf 'Log:         %s\n' "$LOG_FILE"
    printf 'Exit:        %d\n' "$rc"
}

trap on_error ERR
trap on_exit EXIT

resolve_carriers() {
    case "$MODE" in
        tcp)
            SELECTED_CARRIERS="tcp"
            ;;
        udp)
            SELECTED_CARRIERS="udp"
            ;;
        both|benchmark|all)
            SELECTED_CARRIERS="udp tcp"
            ;;
        publish)
            SELECTED_CARRIERS="publish-only"
            ;;
        *)
            die "Unknown mode '$MODE'. Use: tcp|udp|both|all|publish"
            ;;
    esac
}

preflight() {
    CURRENT_PHASE="preflight"

    for cmd in bash git python3 ssh scp tar jq sha256sum rsync; do
        need_cmd "$cmd"
    done

    [[ -f "$PIPELINE" ]] || die "Missing pipeline: $PIPELINE"
    [[ -f "$ROOT/benchmark/run-all-full.sh" ]] || \
        die "Missing benchmark/run-all-full.sh"
    [[ -f "$PUBLISH_TOOL" ]] || \
        die "Missing publish tool: $PUBLISH_TOOL"

    bash -n "$PIPELINE"
    bash -n "$ROOT/benchmark/run-all-full.sh"
    bash -n "$PUBLISH_TOOL"

    if [[ "$MODE" != "publish" ]]; then
        log "Checking Raspberry connectivity"
        ssh \
            -o BatchMode=yes \
            -o ConnectTimeout=10 \
            -o StrictHostKeyChecking=accept-new \
            "${RPI_USER}@${RPI_ADDR}" \
            'set -e
             echo "Raspberry: $(hostname)"
             uname -r
             grep -q "^stcp " /proc/modules
             command -v python3 >/dev/null
             command -v sudo >/dev/null'
    fi

    if [[ "$AUTO_PUBLISH_WEB" == "1" ]]; then
        log "Checking publication target"
        local publish_host="${WEB_DEPLOY_TARGET%%:*}"
        ssh \
            -o BatchMode=yes \
            -o ConnectTimeout=10 \
            -o StrictHostKeyChecking=accept-new \
            "$publish_host" 'echo publication-target-ready' >/dev/null
    fi

    ok "Preflight complete"
}

latest_result() {
    find "$RESULTS_ROOT" \
        -mindepth 1 \
        -maxdepth 1 \
        -type d \
        -name 'full-*' \
        -printf '%T@ %p\n' 2>/dev/null |
    sort -nr |
    head -1 |
    cut -d' ' -f2-
}

run_pipeline() {
    CURRENT_PHASE="benchmark matrix"

    local before
    local pipeline_mode="benchmark"

    before="$(latest_result)"

    if [[ "$BUILD_FIRST" == "1" || "$MODE" == "all" ]]; then
        pipeline_mode="all"
    fi

    log "Running carrier matrix: $SELECTED_CARRIERS"
    log "Clients:   $CLIENTS_LIST"
    log "Payloads:  $PAYLOADS"
    log "Pipelines: $PIPELINES"
    log "Duration:  $DURATION seconds"

    CARRIERS="$SELECTED_CARRIERS" \
    STCP_CARRIERS="$SELECTED_CARRIERS" \
    RPI_ADDR="$RPI_ADDR" \
    RPI_USER="$RPI_USER" \
    RPI_BENCHMARK_DIR="$RPI_BENCHMARK_DIR" \
    WEB_DEPLOY_TARGET="$WEB_DEPLOY_TARGET" \
    DURATION="$DURATION" \
    CLIENTS_LIST="$CLIENTS_LIST" \
    PAYLOADS="$PAYLOADS" \
    PIPELINES="$PIPELINES" \
    VERIFY="$VERIFY" \
    IRQ_METRICS="$IRQ_METRICS" \
    PERF_METRICS="$PERF_METRICS" \
    CONTINUE_ON_ERROR="$CONTINUE_ON_ERROR" \
    SYNC_RPI="$SYNC_RPI" \
    CARRIER_DEBUG="$CARRIER_DEBUG" \
    CLEAN_OLD_RESULTS="$CLEAN_OLD_RESULTS" \
    KEEP_RESULT_RUNS="$KEEP_RESULT_RUNS" \
    AUTO_PUBLISH_WEB=0 \
        bash "$PIPELINE" "$pipeline_mode"

    LAST_RESULT_DIR="$(latest_result)"

    [[ -n "$LAST_RESULT_DIR" && -d "$LAST_RESULT_DIR" ]] || \
        die "No result directory was created"

    if [[ -n "$before" && "$LAST_RESULT_DIR" == "$before" ]]; then
        die "Pipeline did not create a new result directory"
    fi

    printf '%s\n' "$LAST_RESULT_DIR" >"$RESULTS_ROOT/latest-orchestrated.txt"
    ok "Benchmark complete: $LAST_RESULT_DIR"
}

validate_results() {
    CURRENT_PHASE="result validation"

    [[ -f "$LAST_RESULT_DIR/summary.json" ]] || \
        die "Missing summary.json"

    local carrier
    local json_count=0
    local failure_count=0
    local malformed_count=0
    local missing_carriers=()
    local file
    local errors
    local operations
    local details
    local failed_file="$LAST_RESULT_DIR/FAILED-CASES.tsv"
    local malformed_file="$LAST_RESULT_DIR/MALFORMED-CASES.txt"

    : >"$failed_file"
    : >"$malformed_file"

    for carrier in $SELECTED_CARRIERS; do
        if [[ ! -d "$LAST_RESULT_DIR/$carrier" ]]; then
            missing_carriers+=("$carrier")
            continue
        fi

        while IFS= read -r -d '' file; do
            json_count=$((json_count + 1))

            if ! jq -e 'type == "object"' "$file" >/dev/null 2>&1; then
                malformed_count=$((malformed_count + 1))
                printf '%s\n' "$file" >>"$malformed_file"
                continue
            fi

            errors="$(jq -r '(.errors // 0) | tonumber? // 0' "$file")"
            operations="$(jq -r '(.operations // 0) | tonumber? // 0' "$file")"
            details="$(jq -r '(.error_details // []) | length' "$file")"

            if (( errors != 0 || operations <= 0 || details != 0 )); then
                failure_count=$((failure_count + 1))

                printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
                    "$file" \
                    "$(jq -r '.transport // .mode // "unknown"' "$file")" \
                    "$(jq -r '.clients // "?"' "$file")" \
                    "$(jq -r '.payload_bytes // "?"' "$file")" \
                    "$(jq -r '.pipeline // "?"' "$file")" \
                    "$operations" \
                    "$errors" \
                    "$(jq -r '(.error_details // []) | join("; ")' "$file")" \
                    >>"$failed_file"
            fi
        done < <(
            if [[ "$carrier" == "udp" ]]; then
                find "$LAST_RESULT_DIR/udp" \
                    -maxdepth 1 \
                    -type f \
                    -regextype posix-extended \
                    -regex '.*/(udp|tls|stcp-udp)-c[0-9]+-p[0-9]+-q[0-9]+\.json' \
                    -print0
            else
                find "$LAST_RESULT_DIR/tcp" \
                    -maxdepth 1 \
                    -type f \
                    -regextype posix-extended \
                    -regex '.*/(tcp|tls|stcp-tcp)-c[0-9]+-p[0-9]+-q[0-9]+\.json' \
                    -print0
            fi
        )
    done

    if ((${#missing_carriers[@]})); then
        die "Missing carrier result directories: ${missing_carriers[*]}"
    fi

    (( json_count > 0 )) || die "No benchmark case JSON results found"

    if (( malformed_count > 0 )); then
        die "Found $malformed_count malformed benchmark JSON files; see $malformed_file"
    fi

    if (( failure_count > 0 )); then
        warn "Failed benchmark case files: $failure_count"
        die "Result validation failed; dashboards were not published"
    fi

    rm -f "$failed_file" "$malformed_file"

    {
        echo "validated_at=$(date --iso-8601=seconds)"
        echo "selected_carriers=$SELECTED_CARRIERS"
        echo "json_results=$json_count"
        echo "failed_results=$failure_count"
        echo "git_commit=$(git -C "$ROOT" rev-parse HEAD 2>/dev/null || echo unknown)"
    } >"$LAST_RESULT_DIR/ORCHESTRATION-VALIDATION.txt"

    ok "All $json_count benchmark case JSON files passed validation"
}

archive_results() {
    [[ "$AUTO_ARCHIVE" == "1" ]] || return 0

    CURRENT_PHASE="result archive"
    RESULT_ARCHIVE="${LAST_RESULT_DIR}.tar.gz"

    log "Creating result archive"
    tar \
        -C "$(dirname "$LAST_RESULT_DIR")" \
        -czf "$RESULT_ARCHIVE" \
        "$(basename "$LAST_RESULT_DIR")"

    [[ -s "$RESULT_ARCHIVE" ]] || die "Result archive is empty"

    sha256sum "$RESULT_ARCHIVE" >"${RESULT_ARCHIVE}.sha256"
    printf '%s\n' "$RESULT_ARCHIVE" >"$RESULTS_ROOT/latest-orchestrated-archive.txt"

    ok "Archive ready: $RESULT_ARCHIVE"
}

publish_results() {
    CURRENT_PHASE="dashboard publication"

    [[ "$AUTO_PUBLISH_WEB" == "1" ]] || {
        warn "AUTO_PUBLISH_WEB=0; validated results were not published"
        return 0
    }

    log "Generating and publishing dashboards with: $PUBLISH_TOOL"

    local publish_mode
    case "$SELECTED_CARRIERS" in
        "udp")
            publish_mode="udp"
            ;;
        "tcp")
            publish_mode="tcp"
            ;;
        "udp tcp"|"tcp udp")
            publish_mode="both"
            ;;
        *)
            die "Unsupported carrier selection for publication: $SELECTED_CARRIERS"
            ;;
    esac

    RESULT_DIR="$LAST_RESULT_DIR" \
    RESULTS_ROOT="$RESULTS_ROOT" \
    WEB_ROOT="$ROOT/web/benchmarks/raspberry-pi" \
    DEPLOY_TARGET="${WEB_DEPLOY_TARGET%/}" \
    PUBLISH=1 \
        bash "$PUBLISH_TOOL" "$publish_mode"

    local carrier
    for carrier in $SELECTED_CARRIERS; do
        [[ -f "$ROOT/web/benchmarks/raspberry-pi/$carrier/index.html" ]] || \
            die "Generated $carrier dashboard index is missing"
    done

    {
        echo "published_at=$(date --iso-8601=seconds)"
        echo "deploy_target=$WEB_DEPLOY_TARGET"
        echo "publish_tool=$PUBLISH_TOOL"
        echo "publish_mode=$publish_mode"
    } >"$LAST_RESULT_DIR/PUBLISHED.txt"

    ok "Published selected carriers to: $WEB_DEPLOY_TARGET"
}

create_golden() {
    [[ "$AUTO_GOLDEN" == "1" ]] || return 0

    CURRENT_PHASE="golden commit and tag"

    local branch
    local tag
    local commit_message

    branch="$(git -C "$ROOT" symbolic-ref --quiet --short HEAD)" || \
        die "Detached HEAD; refusing golden commit"

    tag="${GOLDEN_TAG_PREFIX}-${STAMP}"
    commit_message="benchmark: publish validated STCP ${SELECTED_CARRIERS} matrix"

    git -C "$ROOT" add -A -- \
        benchmark/results \
        benchmark/pipeline-logs \
        web \
        2>/dev/null || true

    if ! git -C "$ROOT" diff --cached --quiet; then
        git -C "$ROOT" commit -m "$commit_message"
    else
        log "No staged files; tagging current HEAD"
    fi

    git -C "$ROOT" tag -a "$tag" \
        -m "Validated STCP benchmark baseline" \
        -m "Carriers: $SELECTED_CARRIERS
Clients: $CLIENTS_LIST
Payloads: $PAYLOADS
Pipelines: $PIPELINES
Duration: $DURATION
Results: $LAST_RESULT_DIR
Published: $WEB_DEPLOY_TARGET"

    if [[ "$AUTO_PUSH" == "1" ]]; then
        git -C "$ROOT" push "$GIT_REMOTE" "$branch"
        git -C "$ROOT" push "$GIT_REMOTE" "$tag"
    fi

    ok "Golden tag created: $tag"
}

publish_existing() {
    [[ -n "$PUBLISH_RESULT_DIR" ]] || \
        die "Usage: $0 publish /path/to/full-result-directory"

    LAST_RESULT_DIR="$(cd "$PUBLISH_RESULT_DIR" && pwd)"
    case "${CARRIERS:-udp tcp}" in
        udp) SELECTED_CARRIERS="udp" ;;
        tcp) SELECTED_CARRIERS="tcp" ;;
        "udp tcp"|"tcp udp"|both) SELECTED_CARRIERS="udp tcp" ;;
        *) die "Invalid CARRIERS value: ${CARRIERS:-}" ;;
    esac

    validate_results
    archive_results
    publish_results
    create_golden
}

main() {
    cd "$ROOT"
    resolve_carriers
    preflight

    if [[ "$MODE" == "publish" ]]; then
        publish_existing
        return
    fi

    run_pipeline
    validate_results
    archive_results
    publish_results
    create_golden
}

main "$@"
