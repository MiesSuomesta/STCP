#!/usr/bin/env bash
set -Eeuo pipefail
STCP_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
TEST="$STCP_ROOT/application/testing/robot-v2"

: "${STCP_ZEPHYR_SERIAL:=/dev/ttyACM0}"
: "${STCP_ZEPHYR_BAUD:=115200}"
: "${STCP_ZEPHYR_SERVER_HOST:=192.168.1.20}"
: "${STCP_ZEPHYR_SERVER_PORT:=19000}"
: "${STCP_ZEPHYR_TOTAL:=262144}"
: "${STCP_ZEPHYR_CHUNK:=4096}"

command -v robot >/dev/null 2>&1 || {
    echo "[FAIL] Robot Framework missing. Install application/testing/robot-v2/requirements.txt" >&2
    exit 2
}

if [[ -r /proc/net/protocols ]] && ! grep -qi stcp /proc/net/protocols; then
    echo "[WARN] STCP is not visible in /proc/net/protocols; ensure the Linux stcp module is loaded." >&2
fi

make -C "$TEST/server" clean all
export STCP_ZEPHYR_SERVER_BIN="$TEST/server/stcp-v2-bench-server"
export STCP_ZEPHYR_SERIAL STCP_ZEPHYR_BAUD STCP_ZEPHYR_SERVER_HOST STCP_ZEPHYR_SERVER_PORT
export STCP_ZEPHYR_TOTAL STCP_ZEPHYR_CHUNK

RESULT_ROOT="${STCP_ZEPHYR_ROBOT_RESULTS:-$TEST/results}"
RUN_ID="$(date +%Y%m%d-%H%M%S)"
RUN_DIR="$RESULT_ROOT/$RUN_ID"
mkdir -p "$RUN_DIR"

echo "[INFO] Zephyr v2 Robot run: $RUN_DIR"
cd "$TEST"
set +e
robot --outputdir "$RUN_DIR" zephyr-v2.robot
rc=$?
set -e

cp -f server.stdout.log server.stderr.log "$RUN_DIR/" 2>/dev/null || true
ln -sfn "$RUN_DIR" "$RESULT_ROOT/latest"

if (( rc == 0 )); then
    echo "[OK] Zephyr STCPv2 Robot PASS"
else
    echo "[FAIL] Zephyr STCPv2 Robot failed rc=$rc" >&2
fi
exit "$rc"
