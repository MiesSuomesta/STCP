#!/usr/bin/env bash
set -euo pipefail

HOST=raspi
USER=pi

echo "=== Rebooting $HOST ==="
ssh "$USER@$HOST" 'sudo reboot' || true

echo "=== Waiting for host to go down ==="
for _ in $(seq 1 30); do
    if ! ping -c1 -W1 "$HOST" >/dev/null 2>&1; then
        break
    fi
    sleep 1
done

echo "=== Waiting for host to come back ==="
until ping -c1 -W1 "$HOST" >/dev/null 2>&1; do
    sleep 2
done

echo "=== Waiting for SSH ==="
until ssh -o ConnectTimeout=2 -o BatchMode=yes "$USER@$HOST" true 2>/dev/null; do
    sleep 2
done

echo "=== Loading STCP and starting benchmark servers ==="
ssh "$USER@$HOST" '
    set -e
    sudo modprobe stcp
    /home/pi/benchmark/stop-servers.sh || true
    /home/pi/benchmark/start-servers.sh
'

echo "=== Checking listeners ==="
ssh "$USER@$HOST" '
    ss -lnt | grep -E ":(19000|19001|19002)\b" || true
'

echo "=== Server logs ==="
ssh "$USER@$HOST" '
    for f in /home/pi/benchmark/logs/{tcp,tls,stcp}.log; do
        echo "===== $f ====="
        cat "$f" 2>/dev/null || true
    done
'

echo "=== READY ==="
