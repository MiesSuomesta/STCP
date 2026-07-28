#!/usr/bin/env bash
set -Eeuo pipefail

# Run the complete benchmark matrix through run-case.sh.
#
# Usage:
#   run-all.sh tcp|udp|both [RESULT_DIR]
#
# A custom exact matrix can be supplied through CASE_FILE. Format:
#   kind<TAB>clients<TAB>payload<TAB>pipeline<TAB>duration
#
# Without CASE_FILE the matrix is built from environment variables:
#   CLIENTS_LIST="1 4 8 16"
#   PAYLOADS_LIST="64 1024 4096 16384 65536 262144 1048576"
#   PIPELINES_LIST="1 4 8"
#   DURATION=15

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd -P)"

MODE="${1:-both}"
RESULT_DIR="${2:-$SCRIPT_DIR/results/full-$(date +%Y%m%d-%H%M%S)}"

RUN_CASE="${RUN_CASE:-$SCRIPT_DIR/run-case.sh}"
CASE_FILE="${CASE_FILE:-}"

CLIENTS_LIST="${CLIENTS_LIST:-16 8 4 1}"
PAYLOADS_LIST="${PAYLOADS_LIST:-1048576 262144 65536 16384 4096 1024 64}"
PIPELINES_LIST="${PIPELINES_LIST:-8 4 1}"

DURATION="${DURATION:-15}"
CONTINUE_ON_FAILURE="${CONTINUE_ON_FAILURE:-0}"

# Automatic resume is enabled by default. Existing results are accepted only
# when their JSON content matches the selected case and their elapsed time is
# sufficiently close to the configured duration.
AUTO_RESUME="${AUTO_RESUME:-1}"
FORCE_RERUN="${FORCE_RERUN:-0}"
RESUME_MIN_DURATION_RATIO="${RESUME_MIN_DURATION_RATIO:-0.90}"
RESUME_NOTIFY_SKIPS="${RESUME_NOTIFY_SKIPS:-0}"

log()  { printf '[INFO] %s\n' "$*"; }
ok()   { printf '[ OK ] %s\n' "$*"; }
warn() { printf '[WARN] %s\n' "$*" >&2; }
die()  { printf '[FAIL] %s\n' "$*" >&2; exit 1; }


format_hms() {
    local total="${1:-0}"
    local hours minutes seconds

    (( total < 0 )) && total=0

    hours=$((total / 3600))
    minutes=$(((total % 3600) / 60))
    seconds=$((total % 60))

    printf '%02d:%02d:%02d' "$hours" "$minutes" "$seconds"
}

print_progress() {
    local current="$1"
    local total="$2"
    local passed="$3"
    local failed="$4"
    local tuple="$5"
    local send_notification="${6:-1}"
    local now elapsed eta percent

    now="$(date +%s)"
    elapsed=$((now - RUN_STARTED_AT))

    if (( current > 0 )); then
        eta=$(( elapsed * (total - current) / current ))
    else
        eta=0
    fi

    if (( total > 0 )); then
        percent=$(( current * 100 / total ))
    else
        percent=0
    fi

    ELAP="$(format_hms "$elapsed")"
    ETA="$(format_hms "$eta")"

    printf '[PROGRESS] %d/%d => %d%% complete | PASS=%d FAIL=%d | elapsed %s | ETA %s\n\n' \
        "$current" "$total" "$percent" "$passed" "$failed" \
        "$ELAP" "${ETA}"

    if [[ "$send_notification" == 1 ]] && command -v pncnote >/dev/null 2>&1; then
        pncnote             "STCPv2 Benchmark"             "$tuple | ${percent}% | PASS=$passed FAIL=$failed | Elapsed $ELAP | ETA $ETA"             || true
    fi

}


expected_mode_for_kind() {
    case "$1" in
        tcp)      printf 'tcp\n' ;;
        tls)      printf 'tls\n' ;;
        udp)      printf 'udp\n' ;;
        stcp-tcp|stcp-udp) printf 'stcp\n' ;;
        *) return 1 ;;
    esac
}

expected_transport_for_kind() {
    case "$1" in
        tcp|stcp-tcp) printf 'tcp\n' ;;
        tls)          printf 'tls\n' ;;
        udp|stcp-udp) printf 'udp\n' ;;
        *) return 1 ;;
    esac
}

valid_existing_result() {
    local file="$1"
    local kind="$2"
    local clients="$3"
    local payload="$4"
    local pipeline="$5"
    local duration="$6"
    local expected_mode expected_transport

    [[ "$AUTO_RESUME" == 1 ]] || return 1
    [[ "$FORCE_RERUN" != 1 ]] || return 1
    [[ -s "$file" ]] || return 1

    expected_mode="$(expected_mode_for_kind "$kind")" || return 1
    expected_transport="$(expected_transport_for_kind "$kind")" || return 1

    python3 - \
        "$file" \
        "$expected_mode" \
        "$expected_transport" \
        "$clients" \
        "$payload" \
        "$pipeline" \
        "$duration" \
        "$RESUME_MIN_DURATION_RATIO" <<'PY'
import json
import math
import sys
from pathlib import Path

(
    filename,
    expected_mode,
    expected_transport,
    expected_clients,
    expected_payload,
    expected_pipeline,
    expected_duration,
    minimum_ratio,
) = sys.argv[1:]

try:
    value = json.loads(Path(filename).read_text(encoding="utf-8"))
except (OSError, json.JSONDecodeError):
    raise SystemExit(1)

if not isinstance(value, dict):
    raise SystemExit(1)

def integer(name):
    try:
        return int(value.get(name))
    except (TypeError, ValueError):
        raise SystemExit(1)

def number(name):
    try:
        result = float(value.get(name))
    except (TypeError, ValueError):
        raise SystemExit(1)
    if not math.isfinite(result):
        raise SystemExit(1)
    return result

if value.get("mode") != expected_mode:
    raise SystemExit(1)

if value.get("transport") != expected_transport:
    raise SystemExit(1)

if integer("clients") != int(expected_clients):
    raise SystemExit(1)

if integer("payload_bytes") != int(expected_payload):
    raise SystemExit(1)

if integer("pipeline") != int(expected_pipeline):
    raise SystemExit(1)

if integer("errors") != 0:
    raise SystemExit(1)

if integer("operations") <= 0:
    raise SystemExit(1)

if value.get("error_details") not in (None, []):
    raise SystemExit(1)

elapsed = number("elapsed_s")
required = float(expected_duration) * float(minimum_ratio)

if elapsed < required:
    raise SystemExit(1)

raise SystemExit(0)
PY
}

archive_invalid_result() {
    local file="$1"
    local stamp archived

    [[ -e "$file" ]] || return 0

    stamp="$(date +%Y%m%d-%H%M%S)"
    archived="${file}.invalid-${stamp}"

    warn "Archiving invalid/incompatible result: $archived"
    mv -- "$file" "$archived"
}

result_file_for_case() {
    local kind="$1"
    local clients="$2"
    local payload="$3"
    local pipeline="$4"
    local carrier

    case "$kind" in
        tcp|tls|stcp-tcp) carrier=tcp ;;
        udp|stcp-udp)     carrier=udp ;;
        *) return 1 ;;
    esac

    printf '%s/%s/%s-c%s-p%s-q%s.json\n' \
        "$RESULT_DIR" "$carrier" "$kind" "$clients" "$payload" "$pipeline"
}

print_case_result() {
    local file="$1"

    [[ -f "$file" ]] || {
        warn "Missing result JSON: $file"
        return
    }

    echo
    printf '%(%d.%m.%Y %H:%M:%S)T ' -1
    printf '%0.s-' {1..67}
    echo
    echo

    cat "$file"
    echo
}

case "$MODE" in
    tcp)  KINDS=(stcp-tcp tcp tls) ;;
    udp)  KINDS=(stcp-udp udp) ;;
    both) KINDS=(stcp-tcp tls tcp stcp-udp udp) ;;
    *) die "Unknown mode: $MODE" ;;
esac

[[ -x "$RUN_CASE" || -f "$RUN_CASE" ]] || die "Missing run-case script: $RUN_CASE"

RESULT_DIR="$(mkdir -p -- "$RESULT_DIR" && cd -- "$RESULT_DIR" && pwd -P)"
MANIFEST="$RESULT_DIR/cases.tsv"
SUMMARY="$RESULT_DIR/run-summary.tsv"

printf 'kind\tclients\tpayload\tpipeline\tduration\ttuple\n' >"$MANIFEST"
printf 'kind\tclients\tpayload\tpipeline\tduration\tstatus\n' >"$SUMMARY"

emit_cases() {
    local kind clients payload pipeline duration tuple

    if [[ -n "$CASE_FILE" ]]; then
        [[ -f "$CASE_FILE" ]] || die "CASE_FILE not found: $CASE_FILE"

        while IFS=$'\t' read -r kind clients payload pipeline duration; do
            [[ -z "$kind" || "$kind" == \#* || "$kind" == "kind" ]] && continue

            case "$MODE:$kind" in
                tcp:tcp|tcp:tls|tcp:stcp-tcp|udp:udp|udp:stcp-udp|both:*)
                    tuple="Clients ${clients} / Payload ${payload} / Pipes ${pipeline}"
                    printf '%s\t%s\t%s\t%s\t%s\t%s\n' \
                        "$kind" "$clients" "$payload" "$pipeline" \
                        "${duration:-$DURATION}" "$tuple"
                    ;;
            esac
        done <"$CASE_FILE"
        return
    fi

    for clients in $CLIENTS_LIST; do
        for payload in $PAYLOADS_LIST; do
            for pipeline in $PIPELINES_LIST; do
                for kind in "${KINDS[@]}"; do
                    tuple="Clients ${clients} / Payload ${payload} / Pipes ${pipeline}"
                    printf '%s\t%s\t%s\t%s\t%s\t%s\n' \
                        "$kind" "$clients" "$payload" "$pipeline" "$DURATION" "$tuple"
                done
            done
        done
    done
}

mapfile -t CASES < <(emit_cases)
(( ${#CASES[@]} > 0 )) || die "No benchmark cases selected"

printf '%s\n' "${CASES[@]}" >>"$MANIFEST"

log "Mode:       $MODE"
log "Result set: $RESULT_DIR"
log "Cases:      ${#CASES[@]}"
log "Auto resume: $AUTO_RESUME"
log "Force rerun: $FORCE_RERUN"

passed=0
failed=0
resumed=0
executed=0
index=0
RUN_STARTED_AT="$(date +%s)"

for line in "${CASES[@]}"; do
    index=$((index + 1))
    IFS=$'\t' read -r kind clients payload pipeline duration tuple <<<"$line"

    log "Case $index/${#CASES[@]}: $kind c=$clients p=$payload q=$pipeline"

    result_file="$(result_file_for_case "$kind" "$clients" "$payload" "$pipeline")"

    if valid_existing_result         "$result_file"         "$kind"         "$clients"         "$payload"         "$pipeline"         "$duration"
    then
        status=PASS
        passed=$((passed + 1))
        resumed=$((resumed + 1))

        printf '[SKIP] %s-c%s-p%s-q%s already completed and valid
'             "$kind" "$clients" "$payload" "$pipeline"

        print_progress             "$index" "${#CASES[@]}" "$passed" "$failed" "$tuple"             "$RESUME_NOTIFY_SKIPS"
    else
        if [[ -e "$result_file" ]]; then
            archive_invalid_result "$result_file"
        fi

        executed=$((executed + 1))

        if bash "$RUN_CASE"             "$RESULT_DIR"             "$kind"             "$clients"             "$payload"             "$pipeline"             "$duration"
        then
            status=PASS
            passed=$((passed + 1))
            print_case_result "$result_file"
            print_progress                 "$index" "${#CASES[@]}" "$passed" "$failed" "$tuple"
        else
            status=FAIL
            failed=$((failed + 1))
            warn "Case result: $kind c=$clients p=$payload q=$pipeline FAILED"
            print_progress                 "$index" "${#CASES[@]}" "$passed" "$failed" "$tuple"
        fi
    fi

    printf '%s\t%s\t%s\t%s\t%s\t%s\n' \
        "$kind" "$clients" "$payload" "$pipeline" "$duration" "$status" \
        >>"$SUMMARY"

    if [[ "$status" == FAIL && "$CONTINUE_ON_FAILURE" != 1 ]]; then
        die "Stopped at failed case; set CONTINUE_ON_FAILURE=1 to finish the matrix"
    fi
done

cat <<EOF

Benchmark summary
-----------------
Result set: $RESULT_DIR
Passed:     $passed
Failed:     $failed
Resumed:    $resumed
Executed:   $executed
Manifest:   $MANIFEST
Summary:    $SUMMARY
EOF

(( failed == 0 )) || exit 1
ok "All benchmark cases passed"
