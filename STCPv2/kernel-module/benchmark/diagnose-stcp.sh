#!/usr/bin/env bash
set -Eeuo pipefail

HOST="${1:-127.0.0.1}"
PORT="${2:-19002}"

echo '== module =='
lsmod | grep -E '^stcp|curve25519|chacha20poly1305' || true

echo
echo '== module parameters =='
if [[ -r /sys/module/stcp/parameters/carrier_debug ]]; then
    cat /sys/module/stcp/parameters/carrier_debug
else
    echo 'carrier_debug parameter not present'
fi

echo
echo '== benchmark processes =='
ps -ef | grep '[b]enchmark_server.py' || true

echo
echo '== TCP listeners =='
if command -v ss >/dev/null 2>&1; then
    sudo ss -ltnp | grep -E ":(${PORT}|19000|19001)[[:space:]]" || true
else
    echo 'ss not installed'
fi

echo
echo "== TCP carrier reachability ${HOST}:${PORT} =="
python3 - "$HOST" "$PORT" <<'PY'
import socket, sys
host, port = sys.argv[1], int(sys.argv[2])
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.settimeout(2)
try:
    s.connect((host, port))
except Exception as exc:
    print(f"TCP carrier probe failed: {exc!r}")
else:
    print("TCP carrier probe connected")
finally:
    s.close()
PY

echo
echo '== recent STCP kernel log =='
sudo dmesg | grep -i 'stcp:' | tail -n 250 || true
