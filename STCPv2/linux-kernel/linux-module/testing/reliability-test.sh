#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

MODULE="$ROOT/stcp.ko"
TEST="$ROOT/testing/stcp_udp_reliability_test"

load_module() {
    sudo rmmod stcp 2>/dev/null || true

    sudo modprobe libcurve25519
    sudo modprobe libchacha20poly1305

    sudo insmod "$MODULE" "$@" || true

    sleep 1
}

run_case() {
    local name="$1"
    shift

    echo
    echo "========================================="
    echo " RELIABILITY TEST: $name"
    echo "========================================="

    load_module "$@"

    "$TEST"

    sudo rmmod stcp

    echo "[PASS] $name"
}

"$TEST" server &

run_case "baseline"

run_case "drop-first" \
    drop_first_data=1

run_case "loss-10" \
    drop_percent=10

run_case "loss-30" \
    drop_percent=30

run_case "delay-250ms" \
    delay_first_data_ms=250

run_case "duplicate" \
    duplicate_first_data=1

run_case "reorder" \
    reorder_first_pair=1

echo
echo "========================================="
echo " ALL RELIABILITY TESTS PASSED"
echo "========================================="
