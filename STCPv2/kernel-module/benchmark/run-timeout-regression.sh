#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
RUNS="${1:-10}"
CLIENTS="${CLIENTS:-8}"
PAYLOAD="${PAYLOAD:-${PAYLOADS:-64}}"
PIPELINE="${PIPELINE:-${PIPELINES:-4}}"
DURATION="${DURATION:-30}"
FAILURES=0

for ((run = 1; run <= RUNS; run++)); do
    echo "===== STCP timeout regression $run/$RUNS: c=$CLIENTS p=$PAYLOAD q=$PIPELINE ====="

    set +e
    CLIENTS="$CLIENTS" \
    PAYLOADS="$PAYLOAD" \
    PIPELINES="$PIPELINE" \
    DURATION="$DURATION" \
    MODES=stcp \
    bash "$ROOT/run-all.sh" --transport tcp
    status=$?
    set -e

    if ((status != 0)); then
        ((FAILURES += 1))
        echo "[FAIL] regression run $run exited with status $status"
    else
        echo "[ OK ] regression run $run"
    fi
done

echo
if ((FAILURES != 0)); then
    echo "[FAIL] $FAILURES/$RUNS regression runs failed"
    exit 1
fi

echo "[ OK ] all $RUNS regression runs passed"
