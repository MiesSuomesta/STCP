#!/usr/bin/env bash
set -Eeuo pipefail

# Re-run only benchmark parameter combinations whose STCP JSON result failed.
#
# A failed STCP result is one where:
#   - JSON cannot be parsed
#   - errors > 0
#   - error_details is non-empty
#   - operations <= 0
#
# By default the newest benchmark/results/full-* directory is inspected.
# Each failed tuple is rerun with a short duration. The existing benchmark
# pipeline is used, so TCP/TLS baselines may also be rerun for the same tuple.
#
# Usage:
#   ./benchmark/rerun-failed-stcp.sh
#   ./benchmark/rerun-failed-stcp.sh /path/to/full-result
#
# Examples:
#   DURATION=15 ./benchmark/rerun-failed-stcp.sh
#   DURATION=30 MAX_RETRIES=2 ./benchmark/rerun-failed-stcp.sh
#   DRY_RUN=1 ./benchmark/rerun-failed-stcp.sh
#
# Optional environment:
#   DURATION=15
#   MAX_RETRIES=1
#   RPI_ADDR=192.168.1.199
#   RPI_USER=pi
#   PERF_METRICS=0
#   IRQ_METRICS=0
#   VERIFY=0
#   SYNC_RPI=1
#   CONTINUE_ON_ERROR=1
#   AUTO_PUBLISH_WEB=0
#   DRY_RUN=0

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
RESULTS_ROOT="${RESULTS_ROOT:-$SCRIPT_DIR/results}"
PIPELINE="${PIPELINE:-$ROOT/build-benchmark-publish.sh}"

SOURCE_RESULT="${1:-}"
DURATION="${DURATION:-15}"
MAX_RETRIES="${MAX_RETRIES:-1}"
PERF_METRICS="${PERF_METRICS:-0}"
IRQ_METRICS="${IRQ_METRICS:-0}"
VERIFY="${VERIFY:-0}"
SYNC_RPI="${SYNC_RPI:-1}"
CONTINUE_ON_ERROR="${CONTINUE_ON_ERROR:-1}"
AUTO_PUBLISH_WEB="${AUTO_PUBLISH_WEB:-0}"
DRY_RUN="${DRY_RUN:-0}"

STAMP="$(date +%Y%m%d-%H%M%S)"
WORK_DIR="$RESULTS_ROOT/retry-failed-$STAMP"
FAILED_LIST="$WORK_DIR/failed.tsv"
SUMMARY="$WORK_DIR/retry-summary.tsv"

log()  { printf '[INFO] %s\n' "$*"; }
ok()   { printf '[ OK ] %s\n' "$*"; }
warn() { printf '[WARN] %s\n' "$*" >&2; }
die()  { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

latest_result() {
    find "$RESULTS_ROOT" \
        -mindepth 1 -maxdepth 1 \
        -type d -name 'full-*' \
        -printf '%T@ %p\n' 2>/dev/null |
        sort -nr |
        head -1 |
        cut -d' ' -f2-
}

resolve_source_result() {
    local candidate="$SOURCE_RESULT"

    if [[ -z "$candidate" && -f "$RESULTS_ROOT/latest-full.txt" ]]; then
        candidate="$(<"$RESULTS_ROOT/latest-full.txt")"
    fi

    if [[ -z "$candidate" || ! -d "$candidate" ]]; then
        candidate="$(latest_result)"
    fi

    [[ -n "$candidate" && -d "$candidate" ]] ||
        die "No full-* benchmark result directory found under $RESULTS_ROOT"

    SOURCE_RESULT="$(cd "$candidate" && pwd)"
}

find_failed_cases() {
    python3 - "$SOURCE_RESULT" "$FAILED_LIST" <<'PY'
import json
import re
import sys
from pathlib import Path

root = Path(sys.argv[1])
output = Path(sys.argv[2])

name_re = re.compile(
    r"^stcp-(tcp|udp)-c([0-9]+)-p([0-9]+)-q([0-9]+)\.json$"
)

rows = []
for carrier in ("tcp", "udp"):
    result_dir = root / carrier
    if not result_dir.is_dir():
        continue

    for path in sorted(result_dir.glob("stcp-*.json")):
        match = name_re.match(path.name)
        if not match:
            continue

        parsed_carrier, clients, payload, pipeline = match.groups()
        reasons = []

        try:
            data = json.loads(path.read_text())
        except Exception as exc:
            data = {}
            reasons.append(f"invalid-json:{type(exc).__name__}")

        if data:
            errors = data.get("errors", 0)
            details = data.get("error_details") or []
            operations = data.get("operations", 0)

            try:
                if int(errors) > 0:
                    reasons.append(f"errors={errors}")
            except (TypeError, ValueError):
                reasons.append(f"invalid-errors={errors!r}")

            if details:
                reasons.append("error_details=" + repr(details)[:240])

            try:
                if int(operations) <= 0:
                    reasons.append(f"operations={operations}")
            except (TypeError, ValueError):
                reasons.append(f"invalid-operations={operations!r}")

        if reasons:
            rows.append(
                (
                    parsed_carrier,
                    int(clients),
                    int(payload),
                    int(pipeline),
                    ";".join(reasons),
                    str(path),
                )
            )

# Deduplicate exact parameter combinations while keeping deterministic order.
seen = set()
unique_rows = []
for row in rows:
    key = row[:4]
    if key not in seen:
        seen.add(key)
        unique_rows.append(row)

with output.open("w") as fh:
    for row in unique_rows:
        fh.write("\t".join(map(str, row)) + "\n")

print(len(unique_rows))
PY
}

latest_after_timestamp() {
    local start_epoch="$1"
    find "$RESULTS_ROOT" \
        -mindepth 1 -maxdepth 1 \
        -type d -name 'full-*' \
        -newermt "@$start_epoch" \
        -printf '%T@ %p\n' 2>/dev/null |
        sort -nr |
        head -1 |
        cut -d' ' -f2-
}

case_passed() {
    local result_dir="$1"
    local carrier="$2"
    local clients="$3"
    local payload="$4"
    local pipeline="$5"
    local json_file="$result_dir/$carrier/stcp-$carrier-c$clients-p$payload-q$pipeline.json"

    python3 - "$json_file" <<'PY'
import json
import sys
from pathlib import Path

path = Path(sys.argv[1])
if not path.is_file():
    print(f"missing:{path}")
    raise SystemExit(1)

try:
    data = json.loads(path.read_text())
except Exception as exc:
    print(f"invalid-json:{type(exc).__name__}")
    raise SystemExit(1)

errors = data.get("errors", 0)
details = data.get("error_details") or []
operations = data.get("operations", 0)

try:
    passed = int(errors) == 0 and int(operations) > 0 and not details
except (TypeError, ValueError):
    passed = False

if passed:
    print(
        "pass:"
        f"ops={operations},"
        f"ops_s={data.get('operations_s', 'n/a')},"
        f"p99_ms={data.get('rtt_p99_ms', 'n/a')}"
    )
    raise SystemExit(0)

print(
    "failed:"
    f"errors={errors},"
    f"operations={operations},"
    f"details={details!r}"
)
raise SystemExit(1)
PY
}

rerun_case() {
    local carrier="$1"
    local clients="$2"
    local payload="$3"
    local pipeline="$4"
    local reason="$5"
    local source_json="$6"
    local attempt="$7"

    local start_epoch retry_result verification
    start_epoch="$(date +%s)"

    log "Retry $attempt/$MAX_RETRIES: stcp-$carrier-c$clients-p$payload-q$pipeline"
    log "Original failure: $reason"
    log "Source: $source_json"

    if [[ "$DRY_RUN" == "1" ]]; then
        printf '+ DURATION=%q CLIENTS_LIST=%q PAYLOADS=%q PIPELINES=%q ' \
            "$DURATION" "$clients" "$payload" "$pipeline"
        printf 'bash %q benchmark-only\n' "$PIPELINE"
        printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
            "$carrier" "$clients" "$payload" "$pipeline" \
            "$attempt" "dry-run" "-" >> "$SUMMARY"
        return 0
    fi

    # Some run-all-full.sh versions support a carrier selector while older
    # versions simply ignore it. The parameter tuple is always narrowed.
    STCP_CARRIERS="$carrier" \
    CARRIERS="$carrier" \
    DURATION="$DURATION" \
    CLIENTS_LIST="$clients" \
    PAYLOADS="$payload" \
    PIPELINES="$pipeline" \
    PERF_METRICS="$PERF_METRICS" \
    IRQ_METRICS="$IRQ_METRICS" \
    VERIFY="$VERIFY" \
    SYNC_RPI="$SYNC_RPI" \
    CONTINUE_ON_ERROR="$CONTINUE_ON_ERROR" \
    AUTO_PUBLISH_WEB="$AUTO_PUBLISH_WEB" \
        bash "$PIPELINE" benchmark-only || true

    retry_result="$(latest_after_timestamp "$start_epoch")"
    if [[ -z "$retry_result" || ! -d "$retry_result" ]]; then
        warn "Retry produced no full-* result directory"
        printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
            "$carrier" "$clients" "$payload" "$pipeline" \
            "$attempt" "no-result" "-" >> "$SUMMARY"
        return 1
    fi

    if verification="$(case_passed \
        "$retry_result" "$carrier" "$clients" "$payload" "$pipeline")"; then
        ok "Fixed: stcp-$carrier-c$clients-p$payload-q$pipeline ($verification)"
        printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
            "$carrier" "$clients" "$payload" "$pipeline" \
            "$attempt" "pass" "$retry_result" >> "$SUMMARY"
        return 0
    fi

    warn "Still failing: stcp-$carrier-c$clients-p$payload-q$pipeline ($verification)"
    printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
        "$carrier" "$clients" "$payload" "$pipeline" \
        "$attempt" "fail" "$retry_result" >> "$SUMMARY"
    return 1
}

main() {
    command -v python3 >/dev/null || die "python3 is required"
    [[ -f "$PIPELINE" ]] || die "Missing pipeline script: $PIPELINE"
    [[ "$DURATION" =~ ^[0-9]+$ ]] || die "DURATION must be an integer"
    [[ "$MAX_RETRIES" =~ ^[1-9][0-9]*$ ]] || die "MAX_RETRIES must be >= 1"

    mkdir -p "$WORK_DIR"
    : > "$SUMMARY"

    resolve_source_result
    log "Inspecting failed STCP cases in: $SOURCE_RESULT"

    local count
    count="$(find_failed_cases)"

    if [[ "$count" == "0" ]]; then
        ok "No failed STCP JSON results found"
        exit 0
    fi

    log "Found $count failed STCP case(s)"
    column -t -s $'\t' "$FAILED_LIST" 2>/dev/null || cat "$FAILED_LIST"

    local total=0 passed=0 failed=0
    local carrier clients payload pipeline reason source_json
    while IFS=$'\t' read -r \
        carrier clients payload pipeline reason source_json; do

        ((total += 1))
        local success=0 attempt
        for ((attempt = 1; attempt <= MAX_RETRIES; attempt++)); do
            if rerun_case \
                "$carrier" "$clients" "$payload" "$pipeline" \
                "$reason" "$source_json" "$attempt"; then
                success=1
                break
            fi
        done

        if ((success)); then
            ((passed += 1))
        else
            ((failed += 1))
        fi
    done < "$FAILED_LIST"

    echo
    echo "== Failed STCP retry summary =="
    printf 'Source result: %s\n' "$SOURCE_RESULT"
    printf 'Duration:      %s s\n' "$DURATION"
    printf 'Cases:         %d\n' "$total"
    printf 'Passed:        %d\n' "$passed"
    printf 'Still failing: %d\n' "$failed"
    printf 'Details:       %s\n' "$SUMMARY"

    if command -v column >/dev/null; then
        {
            printf 'carrier\tclients\tpayload\tpipeline\tattempt\tstatus\tresult\n'
            cat "$SUMMARY"
        } | column -t -s $'\t'
    else
        cat "$SUMMARY"
    fi

    ((failed == 0))
}

main "$@"
