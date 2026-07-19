#!/bin/bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PYTHON="${PYTHON:-python3}"
CHILD_PID=""

cleanup() {
    local rc=$?
    trap - EXIT INT TERM
    if [[ -n "$CHILD_PID" ]] && kill -0 "$CHILD_PID" 2>/dev/null; then
        kill -TERM "$CHILD_PID" 2>/dev/null || true
        for _ in {1..20}; do
            kill -0 "$CHILD_PID" 2>/dev/null || break
            sleep 0.05
        done
        kill -KILL "$CHILD_PID" 2>/dev/null || true
        wait "$CHILD_PID" 2>/dev/null || true
    fi
    exit "$rc"
}
trap cleanup EXIT INT TERM

run_python() {
    "$PYTHON" "$@" &
    CHILD_PID=$!
    set +e
    wait "$CHILD_PID"
    local rc=$?
    set -e
    CHILD_PID=""
    return "$rc"
}

cd "$SCRIPT_DIR"

# Always establish basic correctness before any stress or throughput run.
run_python ./stcp_smoke.py \
    --port "${STCP_SMOKE_PORT:-7777}" \
    --sizes "${STCP_SMOKE_SIZES:-64,4096,65536}" \
    --timeout "${STCP_SMOKE_TIMEOUT:-5}"

if [[ "${RUN_STRESS:-0}" != "1" ]]; then
    echo
    echo "Functional test passed. Stress tests were skipped."
    echo "Run the full suite with: RUN_STRESS=1 bash run_stress_suite.sh"
    exit 0
fi

echo
echo "=== STCP throughput: 4 clients / 25 s ==="
run_python ./stcp_stress.py \
    --mode throughput \
    --pipeline 1 \
    --port 7780 \
    --clients 4 \
    --payload 262144 \
    --duration 25 \
    --json result-throughput-4.json

echo
echo "=== STCP mixed: 8 clients / 30 s ==="
run_python ./stcp_stress.py \
    --mode mixed \
    --port 7781 \
    --reconnect-every 128 \
    --clients 8 \
    --payload 262144 \
    --duration 30 \
    --json result-mixed-8.json

echo
echo "=== STCP churn: 16 clients / 30 s ==="
run_python ./stcp_stress.py \
    --mode churn \
    --port 7782 \
    --clients 16 \
    --payload 4096 \
    --duration 30 \
    --json result-churn-16.json
