#!/usr/bin/env bash
set -Eeuo pipefail

D="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
HOST="${HOST:-${RPI_HOST:-192.168.1.199}}"
PORT="${PORT:-${STCP_PORT:-19002}}"
RUNS="${RUNS:-20}"
DURATION="${DURATION:-5}"
TIMEOUT="${TIMEOUT:-15}"
PAYLOADS="${PAYLOADS:-393216 458752 524288 1048576}"

failures=0
for payload in $PAYLOADS; do
  for run in $(seq 1 "$RUNS"); do
    echo "===== payload=$payload run=$run/$RUNS ====="
    if ! python3 "$D/benchmark_client.py" \
      --mode stcp \
      --transport udp \
      --host "$HOST" \
      --port "$PORT" \
      --clients 1 \
      --payload "$payload" \
      --pipeline 1 \
      --duration "$DURATION" \
      --timeout "$TIMEOUT"; then
      failures=$((failures + 1))
    fi
  done
done

if (( failures != 0 )); then
  echo "[FAIL] $failures STCP/UDP regression case(s) failed" >&2
  exit 1
fi

echo "[PASS] all STCP/UDP ACK recovery cases completed"
