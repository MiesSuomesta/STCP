#!/usr/bin/env bash
set -euo pipefail

export PATH="$PATH:/home/lja/.cargo/bin"

STCP_BIN="${STCP_BIN:-./target/release/stcp-runner}"
TLS_BIN="${TLS_BIN:-./target/release/tls-runner}"

STCP_ADDR="${STCP_ADDR:-127.0.0.1:7777}"
TLS_ADDR="${TLS_ADDR:-127.0.0.1:7778}"

TLS_CERT="${TLS_CERT:-ignored}"

CLIENTS="${CLIENTS:-5}"
SECONDS="${SECONDS:-20}"
BYTES="${BYTES:-$((100*1024*1024))}"
CHURN_BYTES="${CHURN_BYTES:-$((1*1024*1024))}"
CHUNK="${CHUNK:-$((1*1024*1024))}"

OUTDIR="${OUTDIR:-flame-$(date +%Y%m%d-%H%M%S)}"
mkdir -p "$OUTDIR"

run_flame() {
    local name="$1"
    shift

    echo
    echo "=== FLAMEGRAPH $name ==="

    sudo env PATH="$PATH" flamegraph \
        --output "$OUTDIR/$name.svg" \
        -- "$@"

    echo "Saved: $OUTDIR/$name.svg"
}

run_flame stcp_send_many \
    "$STCP_BIN" send-many "$STCP_ADDR" "$CLIENTS" "$BYTES" "$CHUNK"

run_flame tls_send_many \
    "$TLS_BIN" send-many "$TLS_ADDR" "$TLS_CERT" "$CLIENTS" "$BYTES" "$CHUNK"

run_flame stcp_steady \
    "$STCP_BIN" steady "$STCP_ADDR" "$CLIENTS" "$SECONDS" "$CHUNK"

run_flame tls_steady \
    "$TLS_BIN" steady "$TLS_ADDR" "$TLS_CERT" "$CLIENTS" "$SECONDS" "$CHUNK"

run_flame stcp_churn \
    "$STCP_BIN" churn "$STCP_ADDR" "$CLIENTS" "$SECONDS" "$CHURN_BYTES" "$CHUNK"

run_flame tls_churn \
    "$TLS_BIN" churn "$TLS_ADDR" "$TLS_CERT" "$CLIENTS" "$SECONDS" "$CHURN_BYTES" "$CHUNK"

echo
echo "SVG files saved to: $OUTDIR"
