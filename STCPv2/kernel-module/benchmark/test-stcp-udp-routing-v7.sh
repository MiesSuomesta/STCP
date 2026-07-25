#!/usr/bin/env bash
set -Eeuo pipefail
D="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
HOST="${HOST:-192.168.1.199}"
PORT="${PORT:-19002}"
CLIENTS_LIST="${CLIENTS_LIST:-16 8 4 2 1}"
PAYLOAD="${PAYLOAD:-1048576}"
DURATION="${DURATION:-30}"
TIMEOUT="${TIMEOUT:-40}"

for clients in $CLIENTS_LIST; do
    echo "===== STCP/UDP connection-ID routing v7 clients=$clients payload=$PAYLOAD ====="
    python3 "$D/benchmark_client.py" \
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

echo
echo "Inspect kernel events 312/313 after the run:"
echo "  sudo dmesg | grep -E 'event=(301|308|309|312|313)' | tail -500"
