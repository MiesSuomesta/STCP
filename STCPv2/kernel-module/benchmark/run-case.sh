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
#   NET_IFACE=enp2s0
#   NETWORK_MAX_WAIT=1800          # max 30 min network wait
#   NETWORK_NOTIFY_INTERVAL=60    # mobile status every 1 min
#   NETWORK_POLL_INTERVAL=2
#   NETWORK_STABLE_CHECKS=3
#   NETWORK_DIAG_AFTER=300        # save diagnostics after 5 min
#   NETWORK_NOTIFY_SPOOL=          # default: per-case spool file in result dir

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
VERBOSE="${VERBOSE:-0}"

NET_IFACE="${NET_IFACE:-enp2s0}"
NETWORK_MAX_WAIT="${NETWORK_MAX_WAIT:-1800}"
NETWORK_NOTIFY_INTERVAL="${NETWORK_NOTIFY_INTERVAL:-60}"
NETWORK_POLL_INTERVAL="${NETWORK_POLL_INTERVAL:-2}"
NETWORK_STABLE_CHECKS="${NETWORK_STABLE_CHECKS:-3}"
NETWORK_DIAG_AFTER="${NETWORK_DIAG_AFTER:-300}"
NETWORK_NOTIFY_SPOOL="${NETWORK_NOTIFY_SPOOL:-}"

log()  { printf '[INFO] %s\n' "$*"; }
ok()   { printf '[ OK ] %s\n' "$*"; }
warn() { printf '[WARN] %s\n' "$*" >&2; }
die()  { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

log_note() {
    echo "$*"
    command -v pncnote >/dev/null 2>&1 && pncnote -m -a "STCP Benchmark / Network check" "Network state" "$*" || true
}

log_note_verbose() {
    echo "$*"
    if [ $VERBOSE -eq 1 ]; then
        command -v pncnote >/dev/null 2>&1 && pncnote -m -a "STCP Benchmark / Network check" "Network state" "$*" || true
    fi
}

append_network_notice() {
    local message="$1"
    local spool="${NETWORK_NOTIFY_SPOOL:-}"

    [[ -n "$spool" ]] || return 0
    mkdir -p -- "$(dirname -- "$spool")"
    printf '%s %s\n' "$(date --iso-8601=seconds)" "$message" >>"$spool"
}

send_network_notice_spool() {
    local spool="${NETWORK_NOTIFY_SPOOL:-}"
    local title="${1:-Network wait report}"

    [[ -n "$spool" && -s "$spool" ]] || return 0

    if command -v pncnote >/dev/null 2>&1; then
        # pncnote reads the queued report from the file after connectivity returns.
        if pncnote -m -a "STCP Benchmark / Network check" "$title" --file "$spool"; then
            : >"$spool"
            return 0
        fi
    fi

    warn "[NET] Could not send queued pncnote report; keeping spool: $spool"
    return 0
}

network_diagnostics() {
    local iface="$1"
    local host="$2"

    {
        printf '\n===== NETWORK DIAGNOSTICS %s =====\n' "$(date --iso-8601=seconds)"
        printf 'iface=%s host=%s\n' "$iface" "$host"

        printf '\n--- ip link ---\n'
        ip -details link show dev "$iface" 2>&1 || true

        printf '\n--- carrier / operstate ---\n'
        printf 'carrier='
        cat "/sys/class/net/$iface/carrier" 2>/dev/null || printf 'unknown\n'
        printf 'operstate='
        cat "/sys/class/net/$iface/operstate" 2>/dev/null || printf 'unknown\n'

        printf '\n--- addresses and routes ---\n'
        ip address show dev "$iface" 2>&1 || true
        ip route show 2>&1 || true

        printf '\n--- ping ---\n'
        ping -c3 -W1 "$host" 2>&1 || true

        printf '\n--- recent kernel network messages ---\n'
        dmesg 2>/dev/null |
            grep -Ei 'r8169|link is (up|down)|carrier|enp|eth' |
            tail -50 || true
    } >>"${CASE_LOG:-/dev/stderr}"
}

wait_for_network() {
    local iface="$NET_IFACE"
    local host="$HOST"
    local stable=0
    local started now elapsed next_notice
    local state="initialising"
    local last_state=""
    local diagnostics_saved=0
    local max_minutes=$(( (NETWORK_MAX_WAIT + 59) / 60 ))
    local had_wait=0

    started="$(date +%s)"
    next_notice="$NETWORK_NOTIFY_INTERVAL"

    log_note_verbose "[NET] Checking network: iface=$iface host=$host, max wait ${max_minutes} min"

    while true; do
        now="$(date +%s)"
        elapsed=$((now - started))

        if [[ ! -d "/sys/class/net/$iface" ]]; then
            stable=0
            state="interface $iface missing"
        elif [[ ! -r "/sys/class/net/$iface/carrier" ]]; then
            stable=0
            state="carrier state unavailable on $iface"
        elif [[ "$(cat "/sys/class/net/$iface/carrier" 2>/dev/null || echo 0)" != "1" ]]; then
            stable=0
            state="Ethernet link DOWN on $iface"
        elif ping -c1 -W1 "$host" >/dev/null 2>&1; then

            if (( had_wait == 0 )); then
                log_note_verbose "[NET] OK, no need to wait anything.."
		rm -f "$NETWORK_NOTIFY_SPOOL"
		return 0
            fi

            stable=$((stable + 1))
            state="carrier UP, Raspberry reachable ($stable/$NETWORK_STABLE_CHECKS)"

            if (( stable >= NETWORK_STABLE_CHECKS )); then
                if (( had_wait == 1 )); then
                    local recovered_message="[NET] Network recovered after ${elapsed}s; benchmark continues."
                    printf '%s\n' "$recovered_message"
                    append_network_notice "$recovered_message"
                    send_network_notice_spool "Network recovered"
                else
                    log_note_verbose "[NET] OK: carrier UP and $host reachable."
                fi
                return 0
            fi
        else
            stable=0
            state="carrier UP, but Raspberry $host unreachable"
        fi

        had_wait=1

        # Print and queue every state change locally. Delivery waits for connectivity.
        if [[ "$state" != "$last_state" ]]; then
            local state_message="[NET] $state"
            printf '%s\n' "$state_message"
            append_network_notice "$state_message"
            last_state="$state"
        fi

        # Queue a periodic status line once per configured interval.
        if (( elapsed >= next_notice )); then
            local elapsed_min=$((elapsed / 60))
            local status_message="[NET] Waiting for network: ${elapsed_min}/${max_minutes} min; $state"
            printf '%s\n' "$status_message"
            append_network_notice "$status_message"

            while (( next_notice <= elapsed )); do
                next_notice=$((next_notice + NETWORK_NOTIFY_INTERVAL))
            done
        fi

        # Save one diagnostics block after the configured delay.
        if (( diagnostics_saved == 0 && elapsed >= NETWORK_DIAG_AFTER )); then
            network_diagnostics "$iface" "$host"
            diagnostics_saved=1
            append_network_notice "[NET] Diagnostics saved to ${CASE_LOG:-stderr}."
        fi

        if (( elapsed >= NETWORK_MAX_WAIT )); then
            network_diagnostics "$iface" "$host"
            local timeout_message="[FAIL] Network did not recover within ${max_minutes} min; benchmark case aborted."
            printf '%s\n' "$timeout_message" >&2
            append_network_notice "$timeout_message"

            # This may fail while offline. The queue remains for a later run.
            send_network_notice_spool "Network wait timed out"
            return 1
        fi

        sleep "$NETWORK_POLL_INTERVAL"
    done
}

for value in     "$CLIENTS" "$PAYLOAD" "$PIPELINE" "$DURATION" "$MAX_RETRIES"     "$NETWORK_MAX_WAIT" "$NETWORK_NOTIFY_INTERVAL" "$NETWORK_POLL_INTERVAL"     "$NETWORK_STABLE_CHECKS" "$NETWORK_DIAG_AFTER"; do
    [[ "$value" =~ ^[0-9]+([.][0-9]+)?$ ]] || die "Invalid numeric value: $value"
done

command -v jq >/dev/null || die "jq is required"
command -v python3 >/dev/null || die "python3 is required"
command -v ping >/dev/null || die "ping is required"
command -v ip >/dev/null || die "ip is required"

(( NETWORK_MAX_WAIT > 0 )) || die "NETWORK_MAX_WAIT must be greater than zero"
(( NETWORK_NOTIFY_INTERVAL > 0 )) || die "NETWORK_NOTIFY_INTERVAL must be greater than zero"
(( NETWORK_POLL_INTERVAL > 0 )) || die "NETWORK_POLL_INTERVAL must be greater than zero"
(( NETWORK_STABLE_CHECKS > 0 )) || die "NETWORK_STABLE_CHECKS must be greater than zero"

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
if [[ -z "$NETWORK_NOTIFY_SPOOL" ]]; then
    NETWORK_NOTIFY_SPOOL="$OUT_DIR/.${CASE_NAME}.network-notify.queue"
fi
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

wait_for_network || die "Network preflight failed for $CASE_NAME"
# Flush any report left by an earlier timed-out/offline run.
send_network_notice_spool "Previous network wait report"
restart_servers
for ((attempt=1; attempt<=ATTEMPTS; attempt++)); do
    if (( attempt > 1 )); then
        wait_for_network || die "Network preflight failed before retry for $CASE_NAME"
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
