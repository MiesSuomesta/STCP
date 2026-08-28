#!/usr/bin/env bash
set -euo pipefail
PORT="${STCP_COAP_PORT:-56830}"
IFACE="${STCP_TRACE_IFACE:-any}"
OUT="${1:-handshake-wire-$(date +%Y%m%d-%H%M%S).log}"

echo "[trace] Capturing UDP port $PORT on $IFACE -> $OUT"
echo "[trace] Start Zephyr CoAP now. Stop with Ctrl-C after CONNECTED." 
# -tt prints epoch timestamps, directly comparable with gateway wall_us/1e6.
sudo tcpdump -ni "$IFACE" -tt -l -vv "udp port $PORT" 2>&1 | tee "$OUT"
