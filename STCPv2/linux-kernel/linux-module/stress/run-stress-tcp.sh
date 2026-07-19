#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BIN="$ROOT/stress/stcp-stress"
PROTO="${STCP_TCP_PROTO:-253}"
PORT="${STCP_TCP_PORT:-7777}"
CLIENTS="${STCP_STRESS_CLIENTS:-16}"
ITERATIONS="${STCP_STRESS_ITERATIONS:-1000}"
DURATION="${STCP_STRESS_DURATION:-60}"
PAYLOAD="${STCP_STRESS_PAYLOAD:-4096}"
TIMEOUT="${STCP_STRESS_TIMEOUT:-180}"

cleanup() {
    if [[ -n "${SERVER_PID:-}" ]]; then
        kill "$SERVER_PID" 2>/dev/null || true
        wait "$SERVER_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT INT TERM

"$BIN" server \
    --protocol "$PROTO" \
    --port "$PORT" \
    --payload "$PAYLOAD" &
SERVER_PID=$!

sleep 1

echo "=== STCP TCP churn ==="
timeout "${TIMEOUT}s" "$BIN" churn \
    --protocol "$PROTO" \
    --port "$PORT" \
    --clients "$CLIENTS" \
    --iterations "$ITERATIONS" \
    --payload "$PAYLOAD"

echo "=== STCP TCP steady ==="
timeout "${TIMEOUT}s" "$BIN" steady \
    --protocol "$PROTO" \
    --port "$PORT" \
    --clients "$CLIENTS" \
    --duration "$DURATION" \
    --payload "$PAYLOAD"

echo "STCP TCP stress tests passed"
