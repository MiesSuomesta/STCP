#!/usr/bin/env bash
set -Eeuo pipefail

# Restart benchmark servers on Raspberry Pi over SSH.
# This script does not call any local start/restart wrapper, preventing recursion.
#
# Environment:
#   RPI_HOST=pi@192.168.1.199
#   RPI_BENCHMARK_DIR=/home/pi/benchmark
#   CARRIERS=tcp|udp|both
#   STCP_TRANSPORT=tcp|udp
#   SERVER_RESTART_DELAY=1
#   SSH_OPTS="-o BatchMode=yes -o StrictHostKeyChecking=accept-new"

RPI_HOST="${RPI_HOST:-pi@192.168.1.199}"
RPI_BENCHMARK_DIR="${RPI_BENCHMARK_DIR:-/home/pi/benchmark}"
CARRIERS="${CARRIERS:-both}"
STCP_TRANSPORT="${STCP_TRANSPORT:-tcp}"
SERVER_RESTART_DELAY="${SERVER_RESTART_DELAY:-1}"
SSH_OPTS="${SSH_OPTS:--o BatchMode=yes -o StrictHostKeyChecking=accept-new}"

log() { printf '[CASE-RESTART] %s\n' "$*"; }
die() { printf '[CASE-RESTART][FAIL] %s\n' "$*" >&2; exit 1; }

log "Restarting Raspberry benchmark servers"
log "Host:           $RPI_HOST"
log "Directory:      $RPI_BENCHMARK_DIR"
log "Carriers:       $CARRIERS"
log "STCP transport: $STCP_TRANSPORT"

# shellcheck disable=SC2086
ssh $SSH_OPTS "$RPI_HOST" bash -s -- \
    "$RPI_BENCHMARK_DIR" \
    "$CARRIERS" \
    "$STCP_TRANSPORT" \
    "$SERVER_RESTART_DELAY" <<'REMOTE'
set -Eeuo pipefail

BENCHMARK_DIR="$1"
CARRIERS="$2"
STCP_TRANSPORT="$3"
RESTART_DELAY="$4"

cd "$BENCHMARK_DIR"

if ! lsmod | grep -q '^stcp '; then
    sudo modprobe stcp
fi

# Never call a restart wrapper here. Stop directly and then start exactly once.
pkill -f '[b]enchmark_server.py' 2>/dev/null || true
sleep "$RESTART_DELAY"

if [[ ! -f ./start-servers.sh ]]; then
    echo "[CASE-RESTART][FAIL] Missing remote start-servers.sh in $BENCHMARK_DIR" >&2
    exit 1
fi

CARRIERS="$CARRIERS" \
STCP_TRANSPORT="$STCP_TRANSPORT" \
bash ./start-servers.sh
REMOTE

log "Raspberry benchmark servers restarted"
