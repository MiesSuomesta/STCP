#!/usr/bin/env bash
set -Eeuo pipefail
D="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
for mode in tcp tls stcp; do
  pidfile="$D/run/$mode.pid"
  if [[ -f "$pidfile" ]]; then
    pid="$(cat "$pidfile")"; kill "$pid" 2>/dev/null || true; rm -f "$pidfile"; echo "$mode server stopped"
  fi
done
pkill -f "$D/benchmark_server.py" 2>/dev/null || true
