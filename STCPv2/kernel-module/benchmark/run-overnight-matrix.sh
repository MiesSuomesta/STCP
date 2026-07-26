#!/usr/bin/env bash
#
# STCP extended overnight benchmark matrix.
#
# Expected project layout:
#   kernel-module/build-benchmark-publish.sh
#   kernel-module/benchmark/run-all-full.sh
#
# Run from kernel-module:
#   bash benchmark/run-overnight-matrix.sh
#
# Environment overrides:
#   CLIENTS_LIST="1 2 4 8 16 32 64 128"
#   PAYLOADS="1024 65536 131072 262144 1048576 2097152 4194304 8388608 16777216"
#   PIPELINES="1 4 8"
#   DURATION=20
#   RPI_ADDR=192.168.1.199
#   AUTO_PUBLISH_WEB=0
#

set -uo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT="${ROOT:-$(cd "$SCRIPT_DIR/.." && pwd)}"
RESULTS_ROOT="${RESULTS_ROOT:-$ROOT/benchmark/results}"

CLIENTS_LIST="${CLIENTS_LIST:-1 2 4 8 16 32 64 128}"

# 1024 means 1 KiB. 2048 KiB and 2 MiB are the same value, so 2 MiB is
# included once as 2097152.
PAYLOADS="${PAYLOADS:-1024 65536 131072 262144 1048576 2097152 4194304 8388608 16777216}"
PIPELINES="${PIPELINES:-1 4 8}"
DURATION="${DURATION:-20}"

RPI_ADDR="${RPI_ADDR:-192.168.1.199}"
RPI_USER="${RPI_USER:-pi}"
RPI_BENCHMARK_DIR="${RPI_BENCHMARK_DIR:-/home/pi/benchmark}"

VERIFY="${VERIFY:-0}"
IRQ_METRICS="${IRQ_METRICS:-1}"
PERF_METRICS="${PERF_METRICS:-1}"
CONTINUE_ON_ERROR="${CONTINUE_ON_ERROR:-1}"
SYNC_RPI="${SYNC_RPI:-1}"
CARRIER_DEBUG="${CARRIER_DEBUG:-0}"
AUTO_PUBLISH_WEB="${AUTO_PUBLISH_WEB:-0}"

TIMESTAMP="${TIMESTAMP:-$(date +%Y%m%d-%H%M%S)}"
RUN_LOG="${RUN_LOG:-$ROOT/benchmark/pipeline-logs/overnight-${TIMESTAMP}.log}"
SUMMARY_FILE="${SUMMARY_FILE:-$ROOT/benchmark/results/overnight-${TIMESTAMP}-summary.txt}"

log() {
    printf '[%s] %s\n' "$(date '+%F %T')" "$*"
}

die() {
    log "ERROR: $*"
    exit 1
}

require_file() {
    [[ -f "$1" ]] || die "Required file missing: $1"
}

require_command() {
    command -v "$1" >/dev/null 2>&1 || die "Required command missing: $1"
}

require_file "$ROOT/build-benchmark-publish.sh"
require_file "$ROOT/benchmark/run-all-full.sh"

for cmd in bash git python3 ssh scp tar; do
    require_command "$cmd"
done

mkdir -p "$(dirname "$RUN_LOG")" "$RESULTS_ROOT"

case_count_per_carrier=$(
    python3 - "$CLIENTS_LIST" "$PAYLOADS" "$PIPELINES" <<'PY'
import sys
print(len(sys.argv[1].split()) * len(sys.argv[2].split()) * len(sys.argv[3].split()))
PY
)
total_case_groups=$((case_count_per_carrier * 2))
transport_runs=$((total_case_groups * 3))
estimated_seconds=$((transport_runs * DURATION))
estimated_hours=$(
    python3 - "$estimated_seconds" <<'PY'
import sys
print(f"{int(sys.argv[1]) / 3600:.2f}")
PY
)

cat <<EOF | tee "$SUMMARY_FILE"
STCP extended overnight benchmark
=================================

Started:                 $(date --iso-8601=seconds)
Git commit:              $(git -C "$ROOT" rev-parse HEAD 2>/dev/null || echo unknown)
Host:                    $(hostname)
Kernel:                  $(uname -r)
Raspberry:               ${RPI_USER}@${RPI_ADDR}
Duration per transport:  ${DURATION} seconds
Clients:                 ${CLIENTS_LIST}
Payloads:                ${PAYLOADS}
Pipelines:               ${PIPELINES}
Cases per carrier:       ${case_count_per_carrier}
TCP+UDP case groups:     ${total_case_groups}
Individual transports:  ${transport_runs}
Minimum test time:       ${estimated_hours} hours
EOF

log "Checking Raspberry connectivity"
ssh -o BatchMode=yes -o ConnectTimeout=10 \
    "${RPI_USER}@${RPI_ADDR}" "echo connected: \$(hostname)" ||
    die "Raspberry SSH connectivity failed"

before_latest="$(
    find "$RESULTS_ROOT" -mindepth 1 -maxdepth 1 -type d -name 'full-*' \
        -printf '%T@ %p\n' 2>/dev/null |
    sort -nr |
    head -1 |
    cut -d' ' -f2-
)"

log "Starting extended TCP and UDP benchmark matrices"
set +e
(
    cd "$ROOT" || exit 1

    MODE=benchmark-only \
    DURATION="$DURATION" \
    CLIENTS_LIST="$CLIENTS_LIST" \
    PAYLOADS="$PAYLOADS" \
    PIPELINES="$PIPELINES" \
    RPI_ADDR="$RPI_ADDR" \
    RPI_USER="$RPI_USER" \
    RPI_BENCHMARK_DIR="$RPI_BENCHMARK_DIR" \
    VERIFY="$VERIFY" \
    IRQ_METRICS="$IRQ_METRICS" \
    PERF_METRICS="$PERF_METRICS" \
    CONTINUE_ON_ERROR="$CONTINUE_ON_ERROR" \
    SYNC_RPI="$SYNC_RPI" \
    CARRIER_DEBUG="$CARRIER_DEBUG" \
    AUTO_PUBLISH_WEB="$AUTO_PUBLISH_WEB" \
        bash ./build-benchmark-publish.sh
) 2>&1 | tee -a "$RUN_LOG"
pipeline_rc=${PIPESTATUS[0]}
set -u

after_latest="$(
    find "$RESULTS_ROOT" -mindepth 1 -maxdepth 1 -type d -name 'full-*' \
        -printf '%T@ %p\n' 2>/dev/null |
    sort -nr |
    head -1 |
    cut -d' ' -f2-
)"

if [[ -z "$after_latest" || "$after_latest" == "$before_latest" ]]; then
    log "ERROR: benchmark did not create a new full-* result directory"
    pipeline_rc=1
fi

result_archive=""
if [[ -n "$after_latest" && -d "$after_latest" ]]; then
    result_archive="${after_latest}.tar.gz"

    log "Creating result archive: $result_archive"
    tar -C "$(dirname "$after_latest")" \
        -czf "$result_archive" \
        "$(basename "$after_latest")" || pipeline_rc=1

    {
        echo
        echo "Finished:                 $(date --iso-8601=seconds)"
        echo "Pipeline status:          $pipeline_rc"
        echo "Result directory:         $after_latest"
        echo "Result archive:           $result_archive"
    } | tee -a "$SUMMARY_FILE"

    cp -f "$SUMMARY_FILE" "$after_latest/OVERNIGHT-SUMMARY.txt" || true
    printf '%s\n' "$after_latest" >"$RESULTS_ROOT/latest-overnight.txt"
    printf '%s\n' "$result_archive" >"$RESULTS_ROOT/latest-overnight-archive.txt"
fi

if (( pipeline_rc != 0 )); then
    log "Overnight benchmark finished with failures: status=$pipeline_rc"
    exit "$pipeline_rc"
fi

log "Overnight benchmark completed successfully"
