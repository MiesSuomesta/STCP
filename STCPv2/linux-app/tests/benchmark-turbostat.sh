#!/usr/bin/env bash
set -euo pipefail

STCP_BIN="${STCP_BIN:-./target/release/stcp-runner}"
TLS_BIN="${TLS_BIN:-./target/release/tls-runner}"

STCP_ADDR="${STCP_ADDR:-127.0.0.1:7777}"
TLS_ADDR="${TLS_ADDR:-127.0.0.1:7778}"
TLS_CERT="${TLS_CERT:-ignored}"

CLIENTS="${CLIENTS:-5}"
SECONDS="${SECONDS:-30}"
BYTES="${BYTES:-$((100*1024*1024))}"
CHURN_BYTES="${CHURN_BYTES:-$((1*1024*1024))}"
CHUNK="${CHUNK:-$((1*1024*1024))}"

OUTDIR="${OUTDIR:-turbostat-$(date +%Y%m%d-%H%M%S)}"
mkdir -p "$OUTDIR"

TURBOSTAT_SHOW="${TURBOSTAT_SHOW:-Busy%,Bzy_MHz,IRQ,PkgWatt,CorWatt,RAMWatt,Pkg_J,Cor_J,RAM_J}"

run_turbo() {
    local name="$1"
    shift

    local logfile="$OUTDIR/$name.log"
    local cmdfile="$OUTDIR/$name.cmd"

    echo
    echo "=== TURBOSTAT $name [ CMD: $* ] ==="
    echo "$*" | tee "$cmdfile"

    sudo turbostat \
        --Summary \
        --show "$TURBOSTAT_SHOW" \
        -- "$@" 2>&1 | tee "$logfile"
}

run_turbo stcp_send_many \
    "$STCP_BIN" send-many "$STCP_ADDR" "$CLIENTS" "$BYTES" "$CHUNK"

run_turbo tls_send_many \
    "$TLS_BIN" send-many "$TLS_ADDR" "$TLS_CERT" "$CLIENTS" "$BYTES" "$CHUNK"

run_turbo stcp_steady \
    "$STCP_BIN" steady "$STCP_ADDR" "$CLIENTS" "$SECONDS" "$CHUNK"

run_turbo tls_steady \
    "$TLS_BIN" steady "$TLS_ADDR" "$TLS_CERT" "$CLIENTS" "$SECONDS" "$CHUNK"

run_turbo stcp_churn \
    "$STCP_BIN" churn "$STCP_ADDR" "$CLIENTS" "$SECONDS" "$CHURN_BYTES" "$CHUNK"

run_turbo tls_churn \
    "$TLS_BIN" churn "$TLS_ADDR" "$TLS_CERT" "$CLIENTS" "$SECONDS" "$CHURN_BYTES" "$CHUNK"

echo
echo "Turbostat logs saved to: $OUTDIR"
