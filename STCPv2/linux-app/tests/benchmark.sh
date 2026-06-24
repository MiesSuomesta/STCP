#!/usr/bin/env bash
set -euo pipefail

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

OUTDIR="${OUTDIR:-bench-$(date +%Y%m%d-%H%M%S)}"
mkdir -p "$OUTDIR"

run_test() {
    local name="$1"
    shift

    local logfile="$OUTDIR/$name.log"

    echo
    echo "=== RUN $name [ CMD: $* ]==="
    echo "$*" | tee "$logfile.cmd"

    "$@" | tee "$logfile"
}

extract_value() {
    local file="$1"
    local key="$2"

    grep "$key" "$file" \
        | tail -1 \
        | awk -F ':' '{print $2}' \
        | awk '{print $1}'
}

run_test stcp_send_many \
    "$STCP_BIN" send-many "$STCP_ADDR" "$CLIENTS" "$BYTES" "$CHUNK"

run_test tls_send_many \
    "$TLS_BIN" send-many "$TLS_ADDR" "$TLS_CERT" "$CLIENTS" "$BYTES" "$CHUNK"

run_test stcp_steady \
    "$STCP_BIN" steady "$STCP_ADDR" "$CLIENTS" "$SECONDS" "$CHUNK"

run_test tls_steady \
    "$TLS_BIN" steady "$TLS_ADDR" "$TLS_CERT" "$CLIENTS" "$SECONDS" "$CHUNK"

run_test stcp_churn \
    "$STCP_BIN" churn "$STCP_ADDR" "$CLIENTS" "$SECONDS" "$CHURN_BYTES" "$CHUNK"

run_test tls_churn \
    "$TLS_BIN" churn "$TLS_ADDR" "$TLS_CERT" "$CLIENTS" "$SECONDS" "$CHURN_BYTES" "$CHUNK"

summary_line() {
    local label="$1"
    local file="$2"

    local app
    local echo
    local conns

    app="$(extract_value "$file" "aggregate app speed")"
    echo="$(extract_value "$file" "aggregate echo traffic")"
    conns="$(extract_value "$file" "connections")"

    printf "%-18s app=%10s MB/s  echo=%10s MB/s  conns=%s\n" \
        "$label" "$app" "$echo" "$conns"
}

{
    echo
    echo "=============================="
    echo " STCP vs TLS benchmark summary"
    echo "=============================="
    echo "clients:      $CLIENTS"
    echo "seconds:      $SECONDS"
    echo "bytes:        $BYTES"
    echo "churn bytes:  $CHURN_BYTES"
    echo "chunk:        $CHUNK"
    echo

    summary_line "STCP send-many" "$OUTDIR/stcp_send_many.log"
    summary_line "TLS  send-many" "$OUTDIR/tls_send_many.log"
    echo
    summary_line "STCP steady" "$OUTDIR/stcp_steady.log"
    summary_line "TLS  steady" "$OUTDIR/tls_steady.log"
    echo
    summary_line "STCP churn" "$OUTDIR/stcp_churn.log"
    summary_line "TLS  churn" "$OUTDIR/tls_churn.log"

    echo
    echo "Logs saved to: $OUTDIR"
} | tee "$OUTDIR/summary.txt"
