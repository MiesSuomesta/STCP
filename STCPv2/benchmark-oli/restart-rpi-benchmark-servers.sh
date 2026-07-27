#!/usr/bin/env bash
#
# Restart Raspberry benchmark servers before one benchmark_client.py case.
# Intended to be called by orchestrate-stcp-udp-tests.sh's python3 wrapper.
#

set -Eeuo pipefail

RPI_USER="${RPI_USER:-pi}"
RPI_HOST="${RPI_HOST:-${HOST:-192.168.1.199}}"
RPI_SSH_PORT="${RPI_SSH_PORT:-22}"
RPI_TARGET="${RPI_USER}@${RPI_HOST}"
RPI_BENCH_DIR="${RPI_BENCH_DIR:-/home/${RPI_USER}/stcp-benchmark}"
PORT="${PORT:-19002}"
SERVER_RESTART_DELAY="${SERVER_RESTART_DELAY:-2}"

SSH_OPTS=(
    -p "$RPI_SSH_PORT"
    -o BatchMode=yes
    -o ConnectTimeout=10
    -o ServerAliveInterval=15
    -o ServerAliveCountMax=4
    -o StrictHostKeyChecking=accept-new
)

log() {
    printf '[CASE-RESTART] %s\n' "$*" >&2
}

log "Restarting Raspberry servers at $RPI_TARGET"

ssh "${SSH_OPTS[@]}" "$RPI_TARGET" \
    "RPI_BENCH_DIR='$RPI_BENCH_DIR' PORT='$PORT' SERVER_RESTART_DELAY='$SERVER_RESTART_DELAY' bash -s" <<'REMOTE'
set -Eeuo pipefail

cd "$RPI_BENCH_DIR"

before_file="/tmp/stcp-benchmark-pids-before.$$"
after_file="/tmp/stcp-benchmark-pids-after.$$"
trap 'rm -f "$before_file" "$after_file"' EXIT

pgrep -f '^python3 .*benchmark_server\.py' 2>/dev/null |
    sort -n >"$before_file" || true

if [[ -x ./stop-servers.sh ]]; then
    timeout 15s ./stop-servers.sh >/dev/null 2>&1 || true
fi

pkill -f '^bash ./start-servers\.sh$' 2>/dev/null || true

pids="$(pgrep -f '^python3 .*benchmark_server\.py' 2>/dev/null || true)"
if [[ -n "$pids" ]]; then
    kill $pids 2>/dev/null || true
    sleep 1
fi

pids="$(pgrep -f '^python3 .*benchmark_server\.py' 2>/dev/null || true)"
if [[ -n "$pids" ]]; then
    kill -KILL $pids 2>/dev/null || true
    sleep 1
fi

remaining="$(pgrep -af '^python3 .*benchmark_server\.py' 2>/dev/null || true)"
if [[ -n "$remaining" ]]; then
    echo "[CASE-RESTART][FAIL] Server processes remained after stop:" >&2
    echo "$remaining" >&2
    exit 1
fi

[[ -x ./start-servers.sh ]] || {
    echo "[CASE-RESTART][FAIL] start-servers.sh is missing" >&2
    exit 1
}

# start-servers.sh may remain attached or tail logs, therefore cap its runtime.
set +e
timeout 20s ./start-servers.sh \
    >"logs/start-helper-per-case.log" 2>&1
helper_rc=$?
set -e

if [[ $helper_rc -ne 0 && $helper_rc -ne 124 ]]; then
    echo "[CASE-RESTART][FAIL] start-servers.sh status=$helper_rc" >&2
    tail -100 logs/start-helper-per-case.log >&2 || true
    exit "$helper_rc"
fi

# Ensure the STCP instance is exactly UDP for this UDP orchestrator.
stcp_pids="$(pgrep -f '^python3 .*benchmark_server\.py .*--mode stcp( |$)' 2>/dev/null || true)"
if [[ -n "$stcp_pids" ]]; then
    kill $stcp_pids 2>/dev/null || true
    sleep 1
fi

stcp_pids="$(pgrep -f '^python3 .*benchmark_server\.py .*--mode stcp( |$)' 2>/dev/null || true)"
if [[ -n "$stcp_pids" ]]; then
    kill -KILL $stcp_pids 2>/dev/null || true
fi

nohup python3 ./benchmark_server.py \
    --mode stcp \
    --transport udp \
    --host 0.0.0.0 \
    --port "$PORT" \
    >logs/stcp-udp-per-case.log 2>&1 </dev/null &

echo $! >logs/stcp-udp-per-case.pid

ready=0
for _ in $(seq 1 30); do
    if pgrep -af "^python3 .*benchmark_server\.py .*--mode stcp .*--transport udp .*--port ${PORT}( |$)" \
        >/dev/null
    then
        ready=1
        break
    fi
    sleep 0.2
done

sleep "$SERVER_RESTART_DELAY"

pgrep -f '^python3 .*benchmark_server\.py' 2>/dev/null |
    sort -n >"$after_file" || true

if [[ $ready -ne 1 || ! -s "$after_file" ]]; then
    echo "[CASE-RESTART][FAIL] Servers did not become ready" >&2
    pgrep -af '^python3 .*benchmark_server\.py' >&2 || true
    tail -100 logs/stcp-udp-per-case.log >&2 || true
    exit 1
fi

# Hard proof that at least one PID changed.
if cmp -s "$before_file" "$after_file"; then
    echo "[CASE-RESTART][FAIL] Server PID set did not change" >&2
    echo "Before:" >&2
    cat "$before_file" >&2 || true
    echo "After:" >&2
    cat "$after_file" >&2 || true
    exit 1
fi

echo "[CASE-RESTART] PID set changed successfully" >&2
echo "[CASE-RESTART] Before: $(tr '\n' ' ' <"$before_file")" >&2
echo "[CASE-RESTART] After:  $(tr '\n' ' ' <"$after_file")" >&2
REMOTE
