#!/usr/bin/env bash
set -euo pipefail
STCP_PROJECT_ROOT="${STCP_PROJECT_ROOT:-/srv/stcp-project}"
source "${STCP_SETTINGS:-$STCP_PROJECT_ROOT/settings.sh}"
cd "$STCP_ROOT"
wait_down() {
 echo "[INFO] Waiting Raspberry Pi to go down..."
 for _ in $(seq 1 60); do
  if ! ping -c 1 -W 1 raspi >/dev/null 2>&1; then echo "[OK] Raspberry Pi is down"; return; fi
  sleep 1
 done
 echo "[FAIL] Raspberry Pi did not go down within 60 seconds" >&2; return 1
}
wait_ssh() {
 echo "[INFO] Waiting Raspberry Pi SSH..."
 for _ in $(seq 1 180); do
  if ssh -o BatchMode=yes -o ConnectTimeout=2 -o ConnectionAttempts=1 pi@raspi true >/dev/null 2>&1; then
   echo "[OK] Raspberry Pi SSH is ready"; return
  fi
  sleep 1
 done
 echo "[FAIL] Raspberry Pi SSH did not become ready within 180 seconds" >&2; return 1
}
echo "========== RASPBERRY PI BUILD =========="
bash scripts/build-all.sh rpi
echo "========== RASPBERRY PI INSTALL =========="
bash scripts/install-all.sh rpi
echo "========== RASPBERRY PI REBOOT =========="
ssh pi@raspi 'sudo reboot' || true
wait_down
wait_ssh
echo "[OK] Raspberry Pi build + install + reboot complete"
