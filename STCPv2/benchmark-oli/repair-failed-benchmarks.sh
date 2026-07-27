#!/usr/bin/env bash
#
# repair-failed-benchmarks.sh
#
# Scan an existing full-* result set, rerun only failed benchmark JSON cases,
# overwrite a failed JSON only after a successful retry, and optionally publish.
#
# Usage:
#   ./benchmark/repair-failed-benchmarks.sh both [RESULT_DIR]
#   ./benchmark/repair-failed-benchmarks.sh tcp  [RESULT_DIR]
#   ./benchmark/repair-failed-benchmarks.sh udp  [RESULT_DIR]
#
# Example:
#   bash benchmark/repair-failed-benchmarks.sh both \
#     benchmark/results/full-20260727-184245
#
# Useful environment variables:
#   HOST=192.168.1.199
#   DURATION=15
#   MAX_RETRIES=5
#   RETRY_DELAY=3
#   RESTART_SERVERS=1
#   PUBLISH_AFTER=1
#   DRY_RUN=1
#

set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd)"

MODE="${1:-both}"
RESULT_ARG="${2:-}"

RESULTS_ROOT="${RESULTS_ROOT:-$ROOT/benchmark/results}"
BENCHMARK_CLIENT="${BENCHMARK_CLIENT:-$ROOT/benchmark/benchmark_client.py}"
PUBLISH_TOOL="${PUBLISH_TOOL:-$ROOT/publish-latest-benchmarks.sh}"
RESTART_TOOL="${RESTART_TOOL:-$ROOT/benchmark/restart-rpi-servers.sh}"

HOST="${HOST:-192.168.1.199}"
DURATION="${DURATION:-15}"
TIMEOUT="${TIMEOUT:-30}"
MAX_RETRIES="${MAX_RETRIES:-5}"
RETRY_DELAY="${RETRY_DELAY:-3}"
RESTART_SERVERS="${RESTART_SERVERS:-1}"
SERVER_RESTART_DELAY="${SERVER_RESTART_DELAY:-2}"
PUBLISH_AFTER="${PUBLISH_AFTER:-1}"
DRY_RUN="${DRY_RUN:-0}"

TCP_PORT="${TCP_PORT:-19000}"
TLS_PORT="${TLS_PORT:-19001}"
STCP_PORT="${STCP_PORT:-19002}"
UDP_PORT="${UDP_PORT:-19003}"

STAMP="${STAMP:-$(date +%Y%m%d-%H%M%S)}"
LOG_DIR="${LOG_DIR:-$ROOT/benchmark/pipeline-logs}"
LOG_FILE="${LOG_FILE:-$LOG_DIR/repair-failed-$STAMP.log}"

mkdir -p "$LOG_DIR"
exec > >(tee -a "$LOG_FILE") 2>&1

log()  { printf '[INFO] %s\n' "$*"; }
ok()   { printf '[ OK ] %s\n' "$*"; }
warn() { printf '[WARN] %s\n' "$*" >&2; }
die()  { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

usage() {
    cat <<EOF
Usage:
  $0 tcp  [RESULT_DIR]
  $0 udp  [RESULT_DIR]
  $0 both [RESULT_DIR]

Examples:
  $0 both
  $0 both benchmark/results/full-20260727-184245
  $0 both /home/pomo/git/STCP/STCPv2/kernel-module/benchmark/results/full-20260727-184245
EOF
}

need_cmd() {
    command -v "$1" >/dev/null 2>&1 || die "Missing command: $1"
}

is_failed_json() {
    local file="$1"

    jq -e '
        type == "object" and
        (
            ((.errors // 0) | tonumber? // 0) != 0 or
            ((.operations // 0) | tonumber? // 0) <= 0 or
            ((.error_details // []) | length) != 0
        )
    ' "$file" >/dev/null 2>&1
}

is_passed_json() {
    local file="$1"

    jq -e '
        type == "object" and
        ((.errors // 0) | tonumber? // 0) == 0 and
        ((.operations // 0) | tonumber? // 0) > 0 and
        ((.error_details // []) | length) == 0
    ' "$file" >/dev/null 2>&1
}

latest_result_dir() {
    find "$RESULTS_ROOT" \
        -mindepth 1 -maxdepth 1 \
        -type d -name 'full-*' \
        -printf '%T@ %p\n' 2>/dev/null |
        sort -nr |
        head -1 |
        cut -d' ' -f2-
}

resolve_result_dir() {
    local candidate="$RESULT_ARG"

    if [[ -z "$candidate" ]]; then
        candidate="$(latest_result_dir)"
        [[ -n "$candidate" ]] ||
            die "No full-* result directory found below $RESULTS_ROOT"
        log "Result directory source: latest full-*"
    else
        if [[ "$candidate" != /* ]]; then
            candidate="$ROOT/$candidate"
        fi
        log "Result directory source: command-line argument"
    fi

    RESULT_DIR="$(cd -- "$candidate" 2>/dev/null && pwd -P)" ||
        die "Result directory not found: $candidate"

    [[ -d "$RESULT_DIR" ]] || die "Not a directory: $RESULT_DIR"
}

selected_carriers() {
    case "$MODE" in
        tcp)  printf '%s\n' tcp ;;
        udp)  printf '%s\n' udp ;;
        both) printf '%s\n' tcp udp ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            usage >&2
            die "Unknown mode: $MODE"
            ;;
    esac
}

restart_servers() {
    local carrier="$1"

    [[ "$RESTART_SERVERS" == "1" ]] || return 0

    if [[ -f "$RESTART_TOOL" ]]; then
        log "Restarting Raspberry benchmark servers before $carrier retry"
        CARRIERS="$carrier" STCP_TRANSPORT="$carrier" \
            bash "$RESTART_TOOL"
        sleep "$SERVER_RESTART_DELAY"
    else
        warn "Restart tool missing; continuing without restart: $RESTART_TOOL"
    fi
}

parse_case_name() {
    local filename="$1"
    local stem="${filename%.json}"

    if [[ "$stem" =~ ^(stcp-tcp|stcp-udp|tcp|udp|tls)-c([0-9]+)-p([0-9]+)-q([0-9]+)$ ]]; then
        CASE_KIND="${BASH_REMATCH[1]}"
        CASE_CLIENTS="${BASH_REMATCH[2]}"
        CASE_PAYLOAD="${BASH_REMATCH[3]}"
        CASE_PIPELINE="${BASH_REMATCH[4]}"
        return 0
    fi

    return 1
}

case_command() {
    local kind="$1"
    local clients="$2"
    local payload="$3"
    local pipeline="$4"
    local output="$5"

    local mode transport port

    case "$kind" in
        tcp)
            mode="tcp"
            transport="tcp"
            port="$TCP_PORT"
            ;;
        tls)
            mode="tls"
            transport="tcp"
            port="$TLS_PORT"
            ;;
        udp)
            mode="udp"
            transport="udp"
            port="$UDP_PORT"
            ;;
        stcp-tcp)
            mode="stcp"
            transport="tcp"
            port="$STCP_PORT"
            ;;
        stcp-udp)
            mode="stcp"
            transport="udp"
            port="$STCP_PORT"
            ;;
        *)
            return 1
            ;;
    esac

    CASE_CMD=(
        python3 "$BENCHMARK_CLIENT"
        --mode "$mode"
        --transport "$transport"
        --host "$HOST"
        --port "$port"
        --clients "$clients"
        --payload "$payload"
        --pipeline "$pipeline"
        --duration "$DURATION"
        --timeout "$TIMEOUT"
    )

    # benchmark_client prints JSON to stdout. Keep stderr in the log while
    # capturing stdout into a temporary candidate result.
    CASE_OUTPUT="$output"
}

rerun_one_case() {
    local file="$1"
    local carrier="$2"
    local filename
    local attempt tmp backup

    filename="$(basename -- "$file")"

    if ! parse_case_name "$filename"; then
        warn "Skipping unrecognized case filename: $file"
        return 1
    fi

    log "Failed case: $filename"
    log "Tuple: kind=$CASE_KIND clients=$CASE_CLIENTS payload=$CASE_PAYLOAD pipeline=$CASE_PIPELINE"

    for ((attempt = 1; attempt <= MAX_RETRIES; attempt++)); do
        tmp="${file}.retry-${STAMP}-${attempt}.tmp"
        backup="${file}.failed-${STAMP}.bak"

        restart_servers "$carrier"
        case_command \
            "$CASE_KIND" "$CASE_CLIENTS" "$CASE_PAYLOAD" "$CASE_PIPELINE" "$tmp"

        log "Retry $attempt/$MAX_RETRIES: ${CASE_CMD[*]}"

        if [[ "$DRY_RUN" == "1" ]]; then
            ok "DRY_RUN: would rerun $filename"
            return 0
        fi

        rm -f -- "$tmp"

        if "${CASE_CMD[@]}" >"$tmp"; then
            if is_passed_json "$tmp"; then
                if [[ -f "$file" && ! -e "$backup" ]]; then
                    cp -a -- "$file" "$backup"
                fi

                mkdir -p -- "$(dirname -- "$file")"
                mv -f -- "$tmp" "$file"
                ok "Repaired: $file"
                return 0
            fi

            warn "Retry produced a valid JSON result, but the case still failed"
            jq -r '
                "operations=\(.operations // 0) errors=\(.errors // 0) details=\((.error_details // []) | join("; "))"
            ' "$tmp" 2>/dev/null || true
        else
            warn "Benchmark command exited with an error"
        fi

        rm -f -- "$tmp"

        if (( attempt < MAX_RETRIES )); then
            log "Waiting $RETRY_DELAY second(s) before next retry"
            sleep "$RETRY_DELAY"
        fi
    done

    warn "Could not repair after $MAX_RETRIES retries: $file"
    return 1
}

scan_failed_cases() {
    local carrier="$1"
    local dir="$RESULT_DIR/$carrier"
    local file base main
    local -A emitted=()

    [[ -d "$dir" ]] || {
        warn "Carrier directory missing: $dir"
        return 0
    }

    # 1. Existing primary result JSON files which contain an actual failure.
    while IFS= read -r -d '' file; do
        if is_failed_json "$file"; then
            emitted["$file"]=1
            printf '%s\0' "$file"
        fi
    done < <(
        find "$dir" \
            -maxdepth 1 \
            -type f \
            -regextype posix-extended \
            -regex '.*/(tcp|udp|tls|stcp-tcp|stcp-udp)-c[0-9]+-p[0-9]+-q[0-9]+\.json' \
            -print0
    )

    # 2. Interrupted cases: sidecar files exist, but the primary result JSON
    #    was never written. For example:
    #      tls-c16-p1048576-q1.irq-before.json
    #    without:
    #      tls-c16-p1048576-q1.json
    while IFS= read -r -d '' file; do
        base="$(basename -- "$file")"
        base="${base%%.irq-before.json}"
        base="${base%%.irq-after.json}"
        base="${base%%.perf.csv}"
        base="${base%%.perf.log}"
        main="$dir/$base.json"

        if [[ ! -f "$main" && -z "${emitted[$main]+x}" ]]; then
            emitted["$main"]=1
            printf '%s\0' "$main"
        fi
    done < <(
        find "$dir" \
            -maxdepth 1 \
            -type f \
            -regextype posix-extended \
            -regex '.*/(tcp|udp|tls|stcp-tcp|stcp-udp)-c[0-9]+-p[0-9]+-q[0-9]+\.(irq-before\.json|irq-after\.json|perf\.csv|perf\.log)' \
            -print0
    )
}

write_remaining_failures() {
    local carrier="$1"
    local report="$RESULT_DIR/FAILED-${carrier^^}-CASES.tsv"
    local file count=0

    printf 'file\tclients\tpayload\tpipeline\toperations\terrors\terror_details\n' >"$report"

    while IFS= read -r -d '' file; do
        count=$((count + 1))
        if [[ -f "$file" ]]; then
            printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
                "$file" \
                "$(jq -r '.clients // "?"' "$file")" \
                "$(jq -r '.payload_bytes // "?"' "$file")" \
                "$(jq -r '.pipeline // "?"' "$file")" \
                "$(jq -r '.operations // 0' "$file")" \
                "$(jq -r '.errors // 0' "$file")" \
                "$(jq -r '(.error_details // []) | join("; ")' "$file")" \
                >>"$report"
        else
            local filename
            filename="$(basename -- "$file")"
            if parse_case_name "$filename"; then
                printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\n' \
                    "$file" "$CASE_CLIENTS" "$CASE_PAYLOAD" "$CASE_PIPELINE" \
                    0 1 "primary result JSON missing; interrupted case" \
                    >>"$report"
            else
                printf '%s\t?\t?\t?\t0\t1\tprimary result JSON missing\n' \
                    "$file" >>"$report"
            fi
        fi
    done < <(scan_failed_cases "$carrier")

    if (( count == 0 )); then
        rm -f -- "$report"
        ok "$carrier: no failed benchmark cases remain"
    else
        warn "$carrier: $count failed case(s) remain; see $report"
    fi

    printf '%s\n' "$count"
}

main() {
    (( $# <= 2 )) || {
        usage >&2
        die "Too many arguments"
    }

    for cmd in bash find jq python3 tee; do
        need_cmd "$cmd"
    done

    [[ -f "$BENCHMARK_CLIENT" ]] ||
        die "Benchmark client missing: $BENCHMARK_CLIENT"
    [[ "$MAX_RETRIES" =~ ^[1-9][0-9]*$ ]] ||
        die "MAX_RETRIES must be at least 1"

    resolve_result_dir

    log "Mode:       $MODE"
    log "Result set: $RESULT_DIR"
    log "Host:       $HOST"
    log "Duration:   $DURATION s"
    log "Retries:    $MAX_RETRIES"

    local total=0 repaired=0 unresolved=0
    local carrier file
    local carriers=()

    mapfile -t carriers < <(selected_carriers)

    for carrier in "${carriers[@]}"; do
        log "Scanning $carrier benchmark results"

        while IFS= read -r -d '' file; do
            total=$((total + 1))

            if rerun_one_case "$file" "$carrier"; then
                repaired=$((repaired + 1))
            else
                unresolved=$((unresolved + 1))
            fi
        done < <(scan_failed_cases "$carrier")
    done

    if (( total == 0 )); then
        ok "No failed benchmark cases found"
    fi

    local remaining=0 count
    for carrier in "${carriers[@]}"; do
        count="$(write_remaining_failures "$carrier" | tail -1)"
        remaining=$((remaining + count))
    done

    cat <<EOF

Repair summary
--------------
Result set:  $RESULT_DIR
Found:       $total
Repaired:    $repaired
Unresolved:  $remaining
Log:         $LOG_FILE
EOF

    if (( remaining > 0 )); then
        die "$remaining benchmark case(s) still fail"
    fi

    # A "both" result set must contain both carrier directories. Do not
    # accidentally publish a partial TCP-only or UDP-only run as complete.
    if [[ "$MODE" == "both" ]]; then
        [[ -d "$RESULT_DIR/tcp" ]] ||
            die "Cannot publish mode=both: TCP directory is missing"
        [[ -d "$RESULT_DIR/udp" ]] ||
            die "Cannot publish mode=both: UDP directory is missing"
    fi

    if [[ "$PUBLISH_AFTER" == "1" ]]; then
        [[ -f "$PUBLISH_TOOL" ]] ||
            die "Publish tool missing: $PUBLISH_TOOL"

        log "All selected cases pass; publishing the same result set"
        bash "$PUBLISH_TOOL" "$MODE" "$RESULT_DIR"
    else
        warn "PUBLISH_AFTER=0; publication skipped"
    fi
}

main "$@"
