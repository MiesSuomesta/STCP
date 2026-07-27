#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
cd "$SCRIPT_DIR"

CARRIERS="${CARRIERS:-both}"
STCP_TRANSPORT="${STCP_TRANSPORT:-tcp}"

HOST="${HOST:-0.0.0.0}"

TCP_PORT="${TCP_PORT:-19000}"
TLS_PORT="${TLS_PORT:-19001}"
STCP_PORT="${STCP_PORT:-19002}"
UDP_PORT="${UDP_PORT:-19003}"

PYTHON="${PYTHON:-python3}"
SERVER="${SERVER:-$SCRIPT_DIR/benchmark_server.py}"
LOG_DIR="${LOG_DIR:-$SCRIPT_DIR/logs}"
PID_DIR="${PID_DIR:-$SCRIPT_DIR/pids}"

TLS_CERT="${TLS_CERT:-$SCRIPT_DIR/certs/server.crt}"
TLS_KEY="${TLS_KEY:-$SCRIPT_DIR/certs/server.key}"

log() {
    printf '[SERVER-START] %s\n' "$*"
}

die() {
    printf '[SERVER-START][FAIL] %s\n' "$*" >&2
    exit 1
}

[[ -f "$SERVER" ]] || die "Missing benchmark server: $SERVER"

mkdir -p "$LOG_DIR" "$PID_DIR"

start_server() {
    local name="$1"
    shift

    local pid_file="$PID_DIR/$name.pid"
    local log_file="$LOG_DIR/$name.log"

    if [[ -f "$pid_file" ]]; then
        old_pid="$(cat "$pid_file" 2>/dev/null || true)"

        if [[ -n "$old_pid" ]] && kill -0 "$old_pid" 2>/dev/null; then
            log "$name already running with PID $old_pid"
            return 0
        fi

        rm -f "$pid_file"
    fi

    log "Starting $name"

    nohup "$PYTHON" "$SERVER" "$@" \
        >>"$log_file" 2>&1 &

    pid=$!
    printf '%s\n' "$pid" >"$pid_file"

    sleep 0.5

    if ! kill -0 "$pid" 2>/dev/null; then
        tail -50 "$log_file" >&2 || true
        rm -f "$pid_file"
        die "$name failed to start"
    fi

    log "$name started with PID $pid"
}

start_tcp_servers() {

    [[ -f "$TLS_CERT" ]] || die "Missing TLS certificate: $TLS_CERT"
    [[ -f "$TLS_KEY" ]] || die "Missing TLS private key: $TLS_KEY"

    start_server tcp \
        --mode tcp \
        --host "$HOST" \
        --port "$TCP_PORT"

    start_server tls \
        --mode tls \
        --host "$HOST" \
        --port "$TLS_PORT" \
        --cert "$TLS_CERT" \
        --key "$TLS_KEY"

    start_server stcp-tcp \
        --mode stcp \
        --transport tcp \
        --host "$HOST" \
        --port "$STCP_PORT"
}

start_udp_servers() {
    start_server udp \
        --mode udp \
        --transport udp \
        --host "$HOST" \
        --port "$UDP_PORT"

    start_server tls \
        --mode tls \
        --host "$HOST" \
        --port "$TLS_PORT" \
        --cert "$TLS_CERT" \
        --key "$TLS_KEY"

    start_server stcp-udp \
        --mode stcp \
        --transport udp \
        --host "$HOST" \
        --port "$STCP_PORT"
}

case "$CARRIERS" in
    tcp)
        start_tcp_servers
        ;;
    udp)
        start_udp_servers
        ;;
    both)
        # STCP TCP and STCP UDP use the same default port 19002, so they
        # cannot normally run simultaneously. Select which STCP transport
        # is needed for the current benchmark case.
        start_server tcp \
            --mode tcp \
            --host "$HOST" \
            --port "$TCP_PORT"

        start_server tls \
            --mode tls \
            --host "$HOST" \
            --port "$TLS_PORT"

        start_server udp \
            --mode udp \
            --transport udp \
            --host "$HOST" \
            --port "$UDP_PORT"

        case "$STCP_TRANSPORT" in
            tcp)
                start_server stcp-tcp \
                    --mode stcp \
                    --transport tcp \
                    --host "$HOST" \
                    --port "$STCP_PORT"
                ;;
            udp)
                start_server stcp-udp \
                    --mode stcp \
                    --transport udp \
                    --host "$HOST" \
                    --port "$STCP_PORT"
                ;;
            *)
                die "Unknown STCP_TRANSPORT: $STCP_TRANSPORT"
                ;;
        esac
        ;;
    *)
        die "Unknown CARRIERS value: $CARRIERS"
        ;;
esac

log "Requested benchmark servers are running"
