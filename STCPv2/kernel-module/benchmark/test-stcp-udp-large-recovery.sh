#!/usr/bin/env bash
set -Eeuo pipefail

D="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
HOST="${HOST:-192.168.1.199}"
PORT="${PORT:-19002}"
RUNS="${RUNS:-10}"
DURATION="${DURATION:-10}"
TIMEOUT="${TIMEOUT:-15}"
PAYLOADS="${PAYLOADS:-393216 458752 524288 1048576}"

failures=0
for payload in $PAYLOADS; do
    for run in $(seq 1 "$RUNS"); do
        echo "===== payload=$payload run=$run/$RUNS ====="
        output="$(mktemp)"
        if ! python3 "$D/benchmark_client.py" \
            --mode stcp \
            --transport udp \
            --host "$HOST" \
            --port "$PORT" \
            --clients 1 \
            --payload "$payload" \
            --pipeline 1 \
            --duration "$DURATION" \
            --timeout "$TIMEOUT" \
            --output-json "$output"; then
            failures=$((failures + 1))
        fi
        cat "$output"
        if python3 - "$output" <<'PY'
import json, sys
with open(sys.argv[1], encoding='utf-8') as f:
    result = json.load(f)
raise SystemExit(0 if result.get('errors', 1) == 0 and result.get('operations', 0) > 0 else 1)
PY
        then
            echo "[PASS] payload=$payload run=$run"
        else
            echo "[FAIL] payload=$payload run=$run" >&2
            failures=$((failures + 1))
        fi
        rm -f "$output"
    done
done

if (( failures != 0 )); then
    echo "Large-message recovery regression failed: failures=$failures" >&2
    exit 1
fi

echo "Large-message recovery regression passed"
