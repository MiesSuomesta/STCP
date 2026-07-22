#!/usr/bin/env bash
set -Eeuo pipefail
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:${PATH:-}"
D="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
T="${STCP_TRANSPORT:-tcp}"
TCP_PORT="${TCP_PORT:-19000}"; TLS_PORT="${TLS_PORT:-19001}"; STCP_PORT="${STCP_PORT:-19002}"
[[ "$T" == tcp || "$T" == udp ]] || { echo "STCP_TRANSPORT must be tcp or udp" >&2; exit 2; }
mkdir -p "$D"/{run,logs,certs}
if [[ ! -f "$D/certs/server.crt" || ! -f "$D/certs/server.key" ]]; then
  openssl req -x509 -newkey rsa:2048 -nodes -keyout "$D/certs/server.key" -out "$D/certs/server.crt" -days 3650 -subj /CN=stcp-benchmark
fi
sudo modprobe stcp
start(){
  local mode="$1" port="$2"; shift 2
  local name="$mode"; [[ "$mode" == stcp ]] && name="stcp-$T"
  nohup python3 "$D/benchmark_server.py" --mode "$mode" --transport "$T" --port "$port" "$@" >"$D/logs/$name.log" 2>&1 &
  echo $! >"$D/run/$mode.pid"
  echo "$name server started on port $port (PID $!)"
}
start tcp "$TCP_PORT"
start tls "$TLS_PORT" --cert "$D/certs/server.crt" --key "$D/certs/server.key"
start stcp "$STCP_PORT"
sleep 1
cat "$D"/logs/*.log
