#!/usr/bin/env bash
set -Eeuo pipefail

# Run one benchmark case. The first attempt runs normally; every retry
# restarts the benchmark servers first. A failed result never overwrites
# an earlier successful JSON.
#
# Usage:
#   run-case.sh RESULT_DIR KIND CLIENTS PAYLOAD PIPELINE [DURATION]
#
# KIND:
#   tcp | tls | stcp-tcp | udp | stcp-udp
#
# Environment:
#   HOST=192.168.1.199
#   MAX_RETRIES=5          # retries after the first attempt
#   RETRY_DELAY=3
#   TIMEOUT=30
#   TCP_PORT=19000
#   TLS_PORT=19001
#   STCP_PORT=19002
#   UDP_PORT=19003
#   RESTART_TOOL=benchmark/restart-rpi-servers.sh
#   BENCHMARK_CLIENT=benchmark/benchmark_client.py

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd -P)"
PROJECT_ROOT="$(cd -- "$ROOT/.." && pwd -P)"
WORKSPACE_ROOT="$(cd -- "$PROJECT_ROOT/.." && pwd -P)"

RESULT_DIR="${1:?Usage: $0 RESULT_DIR KIND CLIENTS PAYLOAD PIPELINE [DURATION]}"
KIND="${2:?Missing KIND}"
CLIENTS="${3:?Missing CLIENTS}"
PAYLOAD="${4:?Missing PAYLOAD}"
PIPELINE="${5:?Missing PIPELINE}"
DURATION="${6:-${DURATION:-15}}"

HOST="${HOST:-192.168.1.199}"
MAX_RETRIES="${MAX_RETRIES:-5}"
RETRY_DELAY="${RETRY_DELAY:-3}"
TIMEOUT="${TIMEOUT:-30}"

TCP_PORT="${TCP_PORT:-19000}"
TLS_PORT="${TLS_PORT:-19001}"
STCP_PORT="${STCP_PORT:-19002}"
UDP_PORT="${UDP_PORT:-19003}"

BENCHMARK_CLIENT="${BENCHMARK_CLIENT:-}"
RESTART_TOOL="${RESTART_TOOL:-}"

log()  { printf '[INFO] %s\n' "$*"; }
ok()   { printf '[ OK ] %s\n' "$*"; }
warn() { printf '[WARN] %s\n' "$*" >&2; }
die()  { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

for value in "$CLIENTS" "$PAYLOAD" "$PIPELINE" "$DURATION" "$MAX_RETRIES"; do
    [[ "$value" =~ ^[0-9]+([.][0-9]+)?$ ]] || die "Invalid numeric value: $value"
done

command -v jq >/dev/null || die "jq is required"
command -v python3 >/dev/null || die "python3 is required"

find_project_file() {
    local requested="$1"
    shift
    local candidate

    # Explicit path always wins.
    if [[ -n "$requested" ]]; then
        [[ -f "$requested" ]] || return 1
        realpath -e "$requested"
        return 0
    fi

    # Ordered exact candidates before recursive searching.
    for candidate in "$@"; do
        if [[ -f "$candidate" ]]; then
            realpath -e "$candidate"
            return 0
        fi
    done

    return 1
}

find_benchmark_client() {
    local found=""

    found="$(find_project_file "$BENCHMARK_CLIENT" \
        "$SCRIPT_DIR/benchmark_client.py" \
        "$ROOT/benchmark/benchmark_client.py" \
        "$ROOT/benchmark_client.py" \
        "$ROOT/benchmark/client/benchmark_client.py" \
        "$ROOT/benchmark/tools/benchmark_client.py" \
        "$PWD/benchmark/benchmark_client.py" \
        "$PWD/benchmark_client.py" || true)"

    if [[ -z "$found" ]]; then
        found="$(
            find "$ROOT" "$PROJECT_ROOT" "$WORKSPACE_ROOT" \
                -maxdepth 6 \
                -type d \( \
                    -name .git -o \
                    -name results -o \
                    -name pipeline-logs -o \
                    -name '*old*' -o \
                    -name '*backup*' -o \
                    -name '*bak*' \
                \) -prune -o \
                -type f -name benchmark_client.py -print 2>/dev/null |
            awk '!seen[$0]++' |
            awk '
                /\/kernel-module\/benchmark\// { print "1 " $0; next }
                /\/kernel-module\//           { print "2 " $0; next }
                /\/benchmark-oli\//           { print "3 " $0; next }
                                             { print "4 " $0 }
            ' |
            sort -k1,1n -k2,2 |
            cut -d" " -f2- |
            head -1
        )"
    fi

    [[ -n "$found" ]] ||
        die "benchmark_client.py was not found below: $ROOT, $PROJECT_ROOT or $WORKSPACE_ROOT"

    BENCHMARK_CLIENT="$(realpath -e "$found")"
}

find_restart_tool() {
    local found=""

    found="$(find_project_file "$RESTART_TOOL" \
        "$SCRIPT_DIR/restart-rpi-servers.sh" \
        "$SCRIPT_DIR/restart-servers.sh" \
        "$ROOT/restart-rpi-servers.sh" \
        "$ROOT/restart-servers.sh" \
        "$ROOT/benchmark/restart-rpi-servers.sh" \
        "$ROOT/benchmark/restart-servers.sh" \
        "$ROOT/start-servers.sh" \
        "$ROOT/benchmark/start-servers.sh" || true)"

    if [[ -z "$found" ]]; then
        found="$(
            find "$ROOT" "$PROJECT_ROOT" "$WORKSPACE_ROOT" \
                -maxdepth 6 \
                -type d \( \
                    -name .git -o \
                    -name results -o \
                    -name pipeline-logs -o \
                    -name '*old*' -o \
                    -name '*backup*' -o \
                    -name '*bak*' \
                \) -prune -o \
                -type f \( \
                    -name restart-rpi-servers.sh -o \
                    -name restart-servers.sh -o \
                    -name start-servers.sh \
                \) -print 2>/dev/null |
            awk '!seen[$0]++' |
            awk '
                /\/kernel-module\/benchmark\// { print "1 " $0; next }
                /\/kernel-module\//           { print "2 " $0; next }
                /\/benchmark-oli\//           { print "3 " $0; next }
                                             { print "4 " $0 }
            ' |
            sort -k1,1n -k2,2 |
            cut -d" " -f2- |
            head -1
        )"
    fi

    [[ -n "$found" ]] ||
        die "Server restart script was not found. Set RESTART_TOOL=/exact/path/to/script.sh"

    RESTART_TOOL="$(realpath -e "$found")"
}

find_benchmark_client
find_restart_tool

log "Benchmark client: $BENCHMARK_CLIENT"
log "Restart tool:     $RESTART_TOOL"

case "$KIND" in
    tcp)
        CARRIER=tcp; MODE=tcp; TRANSPORT=tcp; PORT="$TCP_PORT"
        ;;
    tls)
        CARRIER=tcp; MODE=tls; TRANSPORT=tcp; PORT="$TLS_PORT"
        ;;
    stcp-tcp)
        CARRIER=tcp; MODE=stcp; TRANSPORT=tcp; PORT="$STCP_PORT"
        ;;
    udp)
        CARRIER=udp; MODE=udp; TRANSPORT=udp; PORT="$UDP_PORT"
        ;;
    stcp-udp)
        CARRIER=udp; MODE=stcp; TRANSPORT=udp; PORT="$STCP_PORT"
        ;;
    *)
        die "Unknown KIND: $KIND"
        ;;
esac

RESULT_DIR="$(mkdir -p -- "$RESULT_DIR" && cd -- "$RESULT_DIR" && pwd -P)"
OUT_DIR="$RESULT_DIR/$CARRIER"
mkdir -p "$OUT_DIR"

CASE_NAME="${KIND}-c${CLIENTS}-p${PAYLOAD}-q${PIPELINE}"
FINAL_JSON="$OUT_DIR/$CASE_NAME.json"
CASE_LOG="$OUT_DIR/$CASE_NAME.run.log"
ATTEMPTS=$((MAX_RETRIES + 1))

validate_json() {
    local file="$1"
    jq -e '
        type == "object" and
        ((.errors // 0) | tonumber? // 0) == 0 and
        ((.operations // 0) | tonumber? // 0) > 0 and
        ((.error_details // []) | length) == 0
    ' "$file" >/dev/null 2>&1
}

restart_servers() {
    [[ -f "$RESTART_TOOL" ]] ||
        die "Retry required, but restart tool is missing: $RESTART_TOOL"

    log "Restarting servers for retry"
    CARRIERS="$CARRIER" STCP_TRANSPORT="$TRANSPORT" bash "$RESTART_TOOL"
}

{
    printf '\n===== %s %s =====\n' "$(date --iso-8601=seconds)" "$CASE_NAME"
    printf 'result_dir=%s host=%s duration=%s timeout=%s\n' \
        "$RESULT_DIR" "$HOST" "$DURATION" "$TIMEOUT"
} >>"$CASE_LOG"

restart_servers
for ((attempt=1; attempt<=ATTEMPTS; attempt++)); do
    if (( attempt > 1 )); then
        restart_servers
        sleep "$RETRY_DELAY"
    fi

    TMP_JSON="$OUT_DIR/.${CASE_NAME}.attempt-${attempt}.$$.json"
    TMP_ERR="$OUT_DIR/.${CASE_NAME}.attempt-${attempt}.$$.stderr"

    log "$CASE_NAME attempt $attempt/$ATTEMPTS"

    cmd=(
        python3 "$BENCHMARK_CLIENT"
        --mode "$MODE"
        --transport "$TRANSPORT"
        --host "$HOST"
        --port "$PORT"
        --clients "$CLIENTS"
        --payload "$PAYLOAD"
        --pipeline "$PIPELINE"
        --duration "$DURATION"
        --timeout "$TIMEOUT"
    )

    printf '[CMD] ' >>"$CASE_LOG"
    printf '%q ' "${cmd[@]}" >>"$CASE_LOG"
    printf '\n' >>"$CASE_LOG"

    set +e
    "${cmd[@]}" >"$TMP_JSON" 2>"$TMP_ERR"
    rc=$?
    set -e

    cat "$TMP_ERR" >>"$CASE_LOG" || true

    if (( rc == 0 )) && validate_json "$TMP_JSON"; then
        mv -f -- "$TMP_JSON" "$FINAL_JSON"
        rm -f -- "$TMP_ERR"
        ok "$CASE_NAME passed"
        exit 0
    fi

    warn "$CASE_NAME failed on attempt $attempt"
    {
        printf '[FAIL] attempt=%s rc=%s\n' "$attempt" "$rc"
        [[ -s "$TMP_JSON" ]] && cat "$TMP_JSON"
        printf '\n'
    } >>"$CASE_LOG"

    rm -f -- "$TMP_JSON" "$TMP_ERR"
done

die "$CASE_NAME failed after $ATTEMPTS total attempts"
