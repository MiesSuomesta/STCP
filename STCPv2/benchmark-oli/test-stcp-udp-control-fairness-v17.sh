#!/usr/bin/env bash
set -euo pipefail

HOST="${HOST:-192.168.1.199}"
PORT="${PORT:-19002}"
CLIENTS_LIST="${CLIENTS_LIST:-16 8 4 2 1}"
PAYLOAD="${PAYLOAD:-1048576}"
DURATION="${DURATION:-30}"
TIMEOUT="${TIMEOUT:-40}"
CONTROL_BUDGET="${CONTROL_BUDGET:-64}"

if [[ -w /sys/module/stcp/parameters/udp_rx_control_budget ]]; then
    printf '%s\n' "$CONTROL_BUDGET" | sudo tee /sys/module/stcp/parameters/udp_rx_control_budget >/dev/null
fi

for clients in $CLIENTS_LIST; do
    echo "===== STCP/UDP bounded control priority v17 clients=$clients payload=$PAYLOAD control_budget=$CONTROL_BUDGET ====="
    python3 benchmark/benchmark_client.py \
        --mode stcp \
        --transport udp \
        --host "$HOST" \
        --port "$PORT" \
        --clients "$clients" \
        --payload "$PAYLOAD" \
        --pipeline 1 \
        --duration "$DURATION" \
        --timeout "$TIMEOUT"
done
