#!/usr/bin/env bash
set -euo pipefail

STCP_BIN="${STCP_BIN:-./target/release/stcp-runner}"
TLS_BIN="${TLS_BIN:-./target/release/tls-runner}"

STCP_ADDR="${STCP_ADDR:-127.0.0.1:7777}"
TLS_ADDR="${TLS_ADDR:-127.0.0.1:7778}"
TLS_CERT="${TLS_CERT:-ignored}"

CLIENTS="${CLIENTS:-5}"
DURATION="${DURATION:-60}"

PAYLOAD_BYTES="${PAYLOAD_BYTES:-6}"
CHUNK="${CHUNK:-6}"

OUTDIR="${OUTDIR:-perf-small-churn-$(date +%Y%m%d-%H%M%S)}"
mkdir -p "$OUTDIR"

EVENTS="${EVENTS:-task-clock,cycles,instructions,context-switches,cpu-migrations,page-faults,cache-references,cache-misses}"

run_perf() {
    local name="$1"
    shift

    local logfile="$OUTDIR/$name.log"
    local perfraw="$OUTDIR/$name.perf.txt"

    echo
    echo "=== PERF $name [ CMD: $* ] ==="
    echo "$*" | tee "$OUTDIR/$name.cmd"

    perf stat \
        -e "$EVENTS" \
        -o "$perfraw" \
        -- "$@" 2>&1 | tee "$logfile"

    cat "$perfraw" >> "$logfile"
}

extract_summary_value() {
    local file="$1"
    local key="$2"

    grep "$key" "$file" \
        | tail -1 \
        | awk -F ':' '{print $2}' \
        | awk '{print $1}'
}

extract_perf_value() {
    local file="$1"
    local key="$2"

    grep -E "[[:space:]]$key($|[[:space:]])" "$file" \
        | tail -1 \
        | awk '{print $1}' \
        | tr -d ','
}

analyze_one() {
    local label="$1"
    local logfile="$2"

    local conns wall app echo cycles instr task_clock ctx cache_miss

    conns="$(extract_summary_value "$logfile" "connections")"
    wall="$(extract_summary_value "$logfile" "wall time")"
    app="$(extract_summary_value "$logfile" "aggregate app speed")"
    echo="$(extract_summary_value "$logfile" "aggregate echo traffic")"

    cycles="$(extract_perf_value "$logfile" "cycles")"
    instr="$(extract_perf_value "$logfile" "instructions")"
    task_clock="$(extract_perf_value "$logfile" "task-clock")"
    ctx="$(extract_perf_value "$logfile" "context-switches")"
    cache_miss="$(extract_perf_value "$logfile" "cache-misses")"

    awk -v label="$label" \
        -v conns="$conns" \
        -v wall="$wall" \
        -v app="$app" \
        -v echo="$echo" \
        -v cycles="$cycles" \
        -v instr="$instr" \
        -v task="$task_clock" \
        -v ctx="$ctx" \
        -v cmiss="$cache_miss" '
    BEGIN {
        if (conns <= 0 || wall <= 0) {
            printf "%-10s no valid results: conns=%s wall=%s\n", label, conns, wall;
            exit;
        }

        conn_s = conns / wall;
        cyc_conn = cycles / conns;
        instr_conn = instr / conns;
        task_conn = task / conns;
        ctx_conn = ctx / conns;
        cmiss_conn = cmiss / conns;

        printf "%-10s conns=%8.0f  conn/s=%10.2f  app=%8.3f MB/s  echo=%8.3f MB/s\n", label, conns, conn_s, app, echo;
        printf "%-10s cycles/conn=%12.0f  instr/conn=%12.0f  task-ms/conn=%10.4f\n", "", cyc_conn, instr_conn, task_conn;
        printf "%-10s ctx/conn=%15.6f  cache-miss/conn=%12.2f\n", "", ctx_conn, cmiss_conn;
    }'
}

run_perf stcp_small_churn \
    "$STCP_BIN" churn "$STCP_ADDR" "$CLIENTS" "$DURATION" "$PAYLOAD_BYTES" "$CHUNK"

run_perf tls_small_churn \
    "$TLS_BIN" churn "$TLS_ADDR" "$TLS_CERT" "$CLIENTS" "$DURATION" "$PAYLOAD_BYTES" "$CHUNK"

{
    echo
    echo "======================================"
    echo " Small-payload churn perf analysis"
    echo "======================================"
    echo "clients:       $CLIENTS"
    echo "duration:      $DURATION"
    echo "payload bytes: $PAYLOAD_BYTES"
    echo "chunk:         $CHUNK"
    echo "events:        $EVENTS"
    echo

    analyze_one "STCP" "$OUTDIR/stcp_small_churn.log"
    echo
    analyze_one "TLS" "$OUTDIR/tls_small_churn.log"

    echo
    echo "Logs saved to: $OUTDIR"
} | tee "$OUTDIR/summary.txt"
