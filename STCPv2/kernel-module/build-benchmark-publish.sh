#!/usr/bin/env bash
set -Eeuo pipefail

# STCP end-to-end pipeline
#   1. preflight checks
#   2. clean + build x86_64 and Raspberry Pi ARM64 modules
#   3. deploy and verify both modules
#   4. run complete TCP and UDP benchmark matrices
#   5. generate static Raspberry Pi TCP/UDP dashboards
#   6. publish dashboards atomically to stcp.fi
#
# Usage:
#   ./build-benchmark-publish.sh all
#   ./build-benchmark-publish.sh build
#   ./build-benchmark-publish.sh benchmark
#   ./build-benchmark-publish.sh publish /path/to/full-result-directory
#
# Recommended:
#   RPI_ADDR=192.168.1.199 \
#   RPI_USER=pi \
#   RPI_BENCHMARK_DIR=/home/pi/benchmark \
#   WEB_DEPLOY_TARGET='www-data@fuji:/var/www/html/public/stcp.fi/benchmarks/raspberry-pi/' \
#   ./build-benchmark-publish.sh all

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
MODE="${1:-all}"
PUBLISH_RESULT_DIR="${2:-}"

# Raspberry connection. RPI_SSH wins when explicitly supplied.
RPI_USER="${RPI_USER:-pi}"
if [[ -n "${RPI_ADDR:-}" ]]; then
    RPI_ADDR="$RPI_ADDR"
elif [[ -n "${RPI_HOST:-}" ]]; then
    RPI_ADDR="$RPI_HOST"
elif [[ -n "${RPI_SSH:-}" ]]; then
    RPI_ADDR="${RPI_SSH#*@}"
else
    RPI_ADDR="192.168.1.199"
fi
if [[ "$RPI_ADDR" == *@* ]]; then
    RPI_SSH="${RPI_SSH:-$RPI_ADDR}"
    RPI_ADDR="${RPI_ADDR#*@}"
else
    RPI_SSH="${RPI_SSH:-$RPI_USER@$RPI_ADDR}"
fi

SSH_OPTS="${SSH_OPTS:--o BatchMode=yes -o ConnectTimeout=10 -o StrictHostKeyChecking=accept-new}"
RPI_BENCHMARK_DIR="${RPI_BENCHMARK_DIR:-/home/pi/benchmark}"
RPI_REMOTE_DIR="${RPI_REMOTE_DIR:-/tmp/stcp-deploy}"
WEB_DEPLOY_TARGET="${WEB_DEPLOY_TARGET:-www-data@fuji:/var/www/html/public/stcp.fi/benchmarks/raspberry-pi/}"

JOBS="${JOBS:-$(nproc)}"
CROSS_COMPILE="${CROSS_COMPILE:-aarch64-linux-gnu-}"
CARRIER_DEBUG="${CARRIER_DEBUG:-0}"
IRQ_METRICS="${IRQ_METRICS:-1}"
PERF_METRICS="${PERF_METRICS:-1}"
REMOTE_PERF_PREFIX="${REMOTE_PERF_PREFIX:-sudo -n}"
CONTINUE_ON_ERROR="${CONTINUE_ON_ERROR:-1}"
VERIFY="${VERIFY:-0}"
DURATION="${DURATION:-30}"
CLIENTS_LIST="${CLIENTS_LIST:-1 2 4 8}"
PAYLOADS="${PAYLOADS:-64 1024 4096 65536 262144 1048576}"
PIPELINES="${PIPELINES:-1 4 8}"
SYNC_RPI="${SYNC_RPI:-1}"
AUTO_PUBLISH_WEB="${AUTO_PUBLISH_WEB:-1}"
CLEAN_OLD_RESULTS="${CLEAN_OLD_RESULTS:-0}"
KEEP_RESULT_RUNS="${KEEP_RESULT_RUNS:-5}"
DRY_RUN="${DRY_RUN:-0}"

BUILD_DEPLOY="$ROOT/build-and-deploy.sh"
FULL_BENCH="$ROOT/benchmark/run-all-full.sh"
WEB_GENERATOR="$ROOT/benchmark/stcp-raspberry-tcp-generator/generate-all.sh"
RESULTS_ROOT="$ROOT/benchmark/results"
PIPELINE_LOG_DIR="$ROOT/benchmark/pipeline-logs"
STAMP="$(date +%Y%m%d-%H%M%S)"
PIPELINE_LOG="$PIPELINE_LOG_DIR/pipeline-$STAMP.log"
CURRENT_PHASE="startup"
START_SECONDS="$SECONDS"

mkdir -p "$PIPELINE_LOG_DIR"
exec > >(tee -a "$PIPELINE_LOG") 2>&1

log() { printf '[INFO] %s\n' "$*"; }
ok() { printf '[ OK ] %s\n' "$*"; }
warn() { printf '[WARN] %s\n' "$*" >&2; }
die() { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

run() {
    if [[ "$DRY_RUN" == "1" ]]; then
        printf '+'
        printf ' %q' "$@"
        printf '\n'
        return 0
    fi
    "$@"
}

on_error() {
    local rc=$?
    printf '\n[FAIL] Pipeline failed in phase: %s (exit=%d)\n' "$CURRENT_PHASE" "$rc" >&2
    printf '[INFO] Log: %s\n' "$PIPELINE_LOG" >&2
    exit "$rc"
}
trap on_error ERR

on_exit() {
    local rc=$?
    local elapsed=$((SECONDS - START_SECONDS))
    printf '\n== STCP pipeline summary ==\n'
    printf 'Mode:       %s\n' "$MODE"
    printf 'Raspberry:  %s\n' "$RPI_SSH"
    printf 'Elapsed:    %dm %02ds\n' "$((elapsed / 60))" "$((elapsed % 60))"
    printf 'Log:        %s\n' "$PIPELINE_LOG"
    printf 'Exit:       %d\n' "$rc"
}
trap on_exit EXIT

need_cmd() {
    command -v "$1" >/dev/null 2>&1 || die "Missing command: $1"
}

ssh_rpi() {
    # shellcheck disable=SC2086
    run ssh $SSH_OPTS "$RPI_SSH" "$@"
}

preflight() {
    CURRENT_PHASE="preflight"
    log "Running local and remote preflight checks"

    for cmd in bash make python3 ssh scp tar git sha256sum tee; do
        need_cmd "$cmd"
    done

    [[ -x "$BUILD_DEPLOY" || -f "$BUILD_DEPLOY" ]] || die "Missing: $BUILD_DEPLOY"
    [[ -x "$FULL_BENCH" || -f "$FULL_BENCH" ]] || die "Missing: $FULL_BENCH"
    [[ -x "$WEB_GENERATOR" || -f "$WEB_GENERATOR" ]] || die "Missing: $WEB_GENERATOR"

    ssh_rpi "set -e; command -v python3 >/dev/null; command -v sudo >/dev/null; uname -r; test -d /lib/modules/\$(uname -r)" >/dev/null

    if [[ "$PERF_METRICS" == "1" ]]; then
        if ! ssh_rpi "command -v perf >/dev/null && $REMOTE_PERF_PREFIX perf stat -a -- sleep 0.05 >/dev/null 2>&1"; then
            warn "Remote perf is unavailable. Continuing with PERF_METRICS=0."
            PERF_METRICS=0
        fi
    fi

    ok "Preflight complete"
}

collect_metadata() {
    CURRENT_PHASE="metadata"
    log "Collecting benchmark platform metadata"

    if [[ "$DRY_RUN" == "1" ]]; then
        BENCHMARK_KERNEL="${BENCHMARK_KERNEL:-dry-run-kernel}"
        BENCHMARK_COMPILER="${BENCHMARK_COMPILER:-dry-run-compiler}"
        PLATFORM_NAME="${PLATFORM_NAME:-Raspberry Pi (dry run)}"
        GIT_COMMIT="${GIT_COMMIT:-dry-run}"
        export BENCHMARK_KERNEL BENCHMARK_COMPILER PLATFORM_NAME GIT_COMMIT
        return 0
    fi

    BENCHMARK_KERNEL="${BENCHMARK_KERNEL:-$(ssh_rpi 'uname -r' | tr -d '\r' | tail -1)}"
    BENCHMARK_COMPILER="${BENCHMARK_COMPILER:-$(ssh_rpi '(gcc --version 2>/dev/null || cc --version 2>/dev/null) | head -1' | tr -d '\r' | tail -1)}"
    PLATFORM_NAME="${PLATFORM_NAME:-$(ssh_rpi "tr -d '\\0' </proc/device-tree/model 2>/dev/null || echo Raspberry-Pi" | tr -d '\r' | tail -1)}"
    GIT_COMMIT="${GIT_COMMIT:-$(git -C "$ROOT" rev-parse --short HEAD 2>/dev/null || echo unknown)}"

    export BENCHMARK_KERNEL BENCHMARK_COMPILER PLATFORM_NAME GIT_COMMIT
    log "Platform: $PLATFORM_NAME"
    log "Kernel:   $BENCHMARK_KERNEL"
    log "Compiler: $BENCHMARK_COMPILER"
    log "Commit:   $GIT_COMMIT"
}

clean_old_results() {
    [[ "$CLEAN_OLD_RESULTS" == "1" ]] || return 0
    CURRENT_PHASE="result cleanup"
    log "Keeping newest $KEEP_RESULT_RUNS full benchmark result directories"
    [[ -d "$RESULTS_ROOT" ]] || return 0

    mapfile -t old_runs < <(
        find "$RESULTS_ROOT" -mindepth 1 -maxdepth 1 -type d -name 'full-*' -printf '%T@ %p\n' \
            | sort -nr | tail -n +$((KEEP_RESULT_RUNS + 1)) | cut -d' ' -f2-
    )
    if ((${#old_runs[@]})); then
        run rm -rf -- "${old_runs[@]}"
    fi
}

build_and_deploy() {
    CURRENT_PHASE="build and deploy"
    log "Building and deploying x86_64 and Raspberry Pi modules"

    RPI_HOST="$RPI_SSH" \
    RPI_SSH_OPTS="$SSH_OPTS" \
    RPI_BENCHMARK_DIR="$RPI_BENCHMARK_DIR" \
    RPI_REMOTE_DIR="$RPI_REMOTE_DIR" \
    JOBS="$JOBS" \
    CROSS_COMPILE="$CROSS_COMPILE" \
    CARRIER_DEBUG="$CARRIER_DEBUG" \
    STOP_BENCHMARKS=1 \
    DRY_RUN="$DRY_RUN" \
        bash "$BUILD_DEPLOY" all

    [[ "$DRY_RUN" == "1" ]] && return 0

    # Verify that both ends have STCP loaded. The deploy script already performs
    # detailed checks; this is an end-to-end guard before a long benchmark.
    grep -q '^stcp ' /proc/modules || die "Local STCP module is not loaded"
    ssh_rpi "grep -q '^stcp ' /proc/modules" || die "Raspberry STCP module is not loaded"

    local local_hash remote_hash
    local_hash="$(sha256sum "$ROOT/raspberry-kernel-module/stcp.ko" | awk '{print $1}')"
    remote_hash="$(ssh_rpi "sha256sum '$RPI_REMOTE_DIR/stcp.ko'" | tr -d '\r' | awk 'NR == 1 { print $1; exit }')"
    [[ "$local_hash" == "$remote_hash" ]] || die "Raspberry module copy hash mismatch: local=$local_hash remote=$remote_hash"
    ok "Modules built, deployed and verified"
}

run_benchmarks() {
    CURRENT_PHASE="TCP and UDP benchmark matrices"
    log "Starting complete TCP and UDP matrices"

    mkdir -p "$RESULTS_ROOT"
    local result_dir="$RESULTS_ROOT/full-$STAMP"

    RPI_HOST="$RPI_ADDR" \
    RPI_SSH="$RPI_SSH" \
    RPI_BENCHMARK_DIR="$RPI_BENCHMARK_DIR" \
    SSH_OPTS="$SSH_OPTS" \
    FULL_RESULT_DIR="$result_dir" \
    DURATION="$DURATION" \
    CLIENTS_LIST="$CLIENTS_LIST" \
    PAYLOADS="$PAYLOADS" \
    PIPELINES="$PIPELINES" \
    VERIFY="$VERIFY" \
    IRQ_METRICS="$IRQ_METRICS" \
    PERF_METRICS="$PERF_METRICS" \
    REMOTE_PERF_PREFIX="$REMOTE_PERF_PREFIX" \
    CONTINUE_ON_ERROR="$CONTINUE_ON_ERROR" \
    SYNC_RPI="$SYNC_RPI" \
    AUTO_PUBLISH_WEB=0 \
        bash "$FULL_BENCH"

    [[ -f "$result_dir/summary.json" ]] || die "Combined benchmark summary missing: $result_dir/summary.json"
    [[ -d "$result_dir/tcp" ]] || die "TCP result directory missing"
    [[ -d "$result_dir/udp" ]] || die "UDP result directory missing"

    printf '%s\n' "$result_dir" > "$RESULTS_ROOT/latest-full.txt"
    LAST_RESULT_DIR="$result_dir"
    export LAST_RESULT_DIR
    ok "Benchmark matrices complete: $result_dir"
}

resolve_result_dir() {
    local candidate="${1:-}"
    if [[ -n "$candidate" ]]; then
        [[ -d "$candidate" ]] || die "Result directory not found: $candidate"
        printf '%s\n' "$(cd "$candidate" && pwd)"
        return
    fi
    if [[ -f "$RESULTS_ROOT/latest-full.txt" ]]; then
        candidate="$(cat "$RESULTS_ROOT/latest-full.txt")"
    fi
    if [[ -z "$candidate" || ! -d "$candidate" ]]; then
        candidate="$(find "$RESULTS_ROOT" -mindepth 1 -maxdepth 1 -type d -name 'full-*' -printf '%T@ %p\n' 2>/dev/null | sort -nr | head -1 | cut -d' ' -f2-)"
    fi
    [[ -n "$candidate" && -d "$candidate" ]] || die "No full benchmark result directory found"
    printf '%s\n' "$candidate"
}

generate_and_publish() {
    local result_dir
    result_dir="$(resolve_result_dir "${1:-${LAST_RESULT_DIR:-}}")"
    CURRENT_PHASE="website generation"
    log "Generating Raspberry Pi TCP and UDP static dashboards from $result_dir"

    local web_output="$result_dir/web/raspberry-pi"
    AUTO_PUBLISH_WEB=0 \
    PLATFORM_NAME="$PLATFORM_NAME" \
    BENCHMARK_KERNEL="$BENCHMARK_KERNEL" \
    BENCHMARK_COMPILER="$BENCHMARK_COMPILER" \
    GIT_COMMIT="$GIT_COMMIT" \
        bash "$WEB_GENERATOR" "$result_dir/tcp" "$result_dir/udp" "$web_output"

    [[ -f "$web_output/index.html" ]] || die "Combined Raspberry benchmark page was not generated"
    [[ -f "$web_output/tcp/index.html" ]] || die "TCP dashboard was not generated"
    [[ -f "$web_output/udp/index.html" ]] || die "UDP dashboard was not generated"

    if [[ "$AUTO_PUBLISH_WEB" == "1" ]]; then
        CURRENT_PHASE="website publication"
        log "Publishing dashboards atomically to $WEB_DEPLOY_TARGET"
        WEB_DEPLOY_TARGET="$WEB_DEPLOY_TARGET" \
            bash "$ROOT/benchmark/stcp-raspberry-tcp-generator/publish-all.sh" "$web_output"
        ok "Published: $WEB_DEPLOY_TARGET"
    else
        warn "AUTO_PUBLISH_WEB=0; generated site was not uploaded"
    fi

    printf '%s\n' "$web_output" > "$RESULTS_ROOT/latest-web.txt"
    ok "Static site ready: $web_output"
}


validate_web_target() {
    [[ "$WEB_DEPLOY_TARGET" == *:* ]] ||
        die "Invalid WEB_DEPLOY_TARGET: $WEB_DEPLOY_TARGET"

    local remote_host="${WEB_DEPLOY_TARGET%%:*}"
    local remote_path="${WEB_DEPLOY_TARGET#*:}"

    [[ -n "$remote_host" && -n "$remote_path" ]] ||
        die "Invalid WEB_DEPLOY_TARGET: $WEB_DEPLOY_TARGET"

    [[ "$remote_path" == /* ]] ||
        die "WEB_DEPLOY_TARGET must use an absolute remote path: $remote_path"

    #if [[ "$remote_host" == www-data@* ]]; then
    #    die "Do not publish through www-data SSH account; use a normal deploy user"
    #fi
}

print_configuration() {
    cat <<EOF_CONFIG
== STCP build, benchmark and publish ==
Mode:                $MODE
Project:             $ROOT
Raspberry SSH:       $RPI_SSH
Raspberry address:   $RPI_ADDR
Benchmark directory: $RPI_BENCHMARK_DIR
Duration:            $DURATION s
Clients:             $CLIENTS_LIST
Payloads:            $PAYLOADS
Pipelines:            $PIPELINES
IRQ metrics:         $IRQ_METRICS
Perf metrics:        $PERF_METRICS
Continue on error:   $CONTINUE_ON_ERROR
Publish:             $AUTO_PUBLISH_WEB
Deploy target:       $WEB_DEPLOY_TARGET
Dry run:             $DRY_RUN
EOF_CONFIG
}

validate_web_target
print_configuration
preflight
collect_metadata
clean_old_results

case "$MODE" in
    all)
        build_and_deploy
        run_benchmarks
        generate_and_publish "$LAST_RESULT_DIR"
        ;;
    build)
        build_and_deploy
        ;;
    benchmark)
        run_benchmarks
        generate_and_publish "$LAST_RESULT_DIR"
        ;;
    benchmark-only)
        run_benchmarks
        ;;
    publish)
        generate_and_publish "$PUBLISH_RESULT_DIR"
        ;;
    generate)
        AUTO_PUBLISH_WEB=0
        generate_and_publish "$PUBLISH_RESULT_DIR"
        ;;
    preflight)
        ;;
    *)
        die "Unknown mode '$MODE'. Use: all|build|benchmark|benchmark-only|publish|generate|preflight"
        ;;
esac
