#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
RUNS="${1:-10}"
CLIENTS="${CLIENTS:-2}"
PAYLOAD="${PAYLOAD:-64}"
PIPELINE="${PIPELINE:-1}"
DURATION="${DURATION:-30}"

: "${RPI_HOST:=192.168.1.199}"
: "${RPI_SSH:=pi@192.168.1.199}"
: "${RPI_BENCHMARK_DIR:=/home/pi/benchmark}"

failures=0
for ((i=1; i<=RUNS; i++)); do
    echo "===== STCP timeout regression $i/$RUNS: c${CLIENTS} p${PAYLOAD} q${PIPELINE} ====="
    set +e
    CLIENTS="$CLIENTS" \
    PAYLOADS="$PAYLOAD" \
    PIPELINES="$PIPELINE" \
    DURATION="$DURATION" \
    RPI_HOST="$RPI_HOST" \
    RPI_SSH="$RPI_SSH" \
    RPI_BENCHMARK_DIR="$RPI_BENCHMARK_DIR" \
    IRQ_METRICS=0 \
    PERF_METRICS=0 \
    "$ROOT/run-all.sh" --transport tcp --only stcp
    status=$?
    set -e
    if (( status != 0 )); then
        failures=$((failures + 1))
    fi
done

echo "Regression runs: $RUNS, failures: $failures"
(( failures == 0 ))
