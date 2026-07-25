#!/usr/bin/env bash
set -Eeuo pipefail

HOST="${HOST:-192.168.1.199}"
PORT="${PORT:-19002}"
DURATION="${DURATION:-30}"
TIMEOUT="${TIMEOUT:-45}"
PAYLOAD="${PAYLOAD:-1048576}"
CLIENTS_LIST="${CLIENTS_LIST:-1 2 4 8 16}"
PIPELINE="${PIPELINE:-1}"
D="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

failures=0
for clients in $CLIENTS_LIST; do
    echo "===== STCP/UDP shared-RX clients=$clients payload=$PAYLOAD ====="
    if ! python3 "$D/benchmark_client.py" \
        --mode stcp \
        --transport udp \
        --host "$HOST" \
        --port "$PORT" \
        --clients "$clients" \
        --payload "$PAYLOAD" \
        --pipeline "$PIPELINE" \
        --duration "$DURATION" \
        --timeout "$TIMEOUT"; then
        failures=$((failures + 1))
    fi
done

if (( failures != 0 )); then
    echo "[FAIL] $failures shared-RX case(s) failed" >&2
    exit 1
fi

echo "[PASS] all shared-RX cases completed"
