#!/usr/bin/env bash
set -euo pipefail
D="$(cd "$(dirname "$0")" && pwd)"; mkdir -p "$D"/{run,logs,certs}
[ -f "$D/certs/server.crt" ] || openssl req -x509 -newkey rsa:2048 -nodes -keyout "$D/certs/server.key" -out "$D/certs/server.crt" -days 3650 -subj /CN=stcp-benchmark
sudo modprobe stcp
start(){ m=$1;p=$2;shift 2; nohup python3 "$D/benchmark_server.py" --mode "$m" --port "$p" "$@" >"$D/logs/$m.log" 2>&1 & echo $! >"$D/run/$m.pid"; }
start tcp 19000
start tls 19001 --cert "$D/certs/server.crt" --key "$D/certs/server.key"
start stcp 19002
sleep 1; cat "$D"/logs/*.log
