#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PYTHON="${PYTHON:-python3}"
CHILD_PID=""

cleanup() {
    local status=$?
    trap - INT TERM EXIT

    if [[ -n "${CHILD_PID}" ]] && kill -0 "${CHILD_PID}" 2>/dev/null; then
        kill -TERM "${CHILD_PID}" 2>/dev/null || true

        for _ in {1..50}; do
            kill -0 "${CHILD_PID}" 2>/dev/null || break
            sleep 0.1
        done

        if kill -0 "${CHILD_PID}" 2>/dev/null; then
            kill -KILL "${CHILD_PID}" 2>/dev/null || true
        fi

        wait "${CHILD_PID}" 2>/dev/null || true
    fi

    exit "${status}"
}

trap cleanup INT TERM EXIT
cd "$SCRIPT_DIR"

run_test() {
    "$PYTHON" ./stcp_stress.py "$@" &
    CHILD_PID=$!
    wait "$CHILD_PID"
    local status=$?
    CHILD_PID=""
    return "$status"
}

echo "=== STCP throughput: 4 clients / 25 s ==="
run_test \
    --mode throughput \
    --pipeline 1 \
    --port 7777 \
    --clients 4 \
    --payload 262144 \
    --duration 25 \
    --json result-throughput-4.json

echo
echo "=== STCP mixed: 8 clients / 30 s ==="
run_test \
    --mode mixed \
    --port 7778 \
    --reconnect-every 128 \
    --clients 8 \
    --payload 262144 \
    --duration 30 \
    --json result-mixed-8.json

echo
echo "=== STCP churn: 16 clients / 30 s ==="
run_test \
    --mode churn \
    --port 7779 \
    --clients 16 \
    --payload 4096 \
    --duration 30 \
    --json result-churn-16.json
