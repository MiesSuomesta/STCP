#!/usr/bin/env bash
set -Eeuo pipefail
APP_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
TEST="$APP_ROOT/testing/robot-app"
GW="$APP_ROOT/testing/gw"

command -v robot >/dev/null 2>&1 || { echo "[FAIL] robot missing" >&2; exit 2; }
command -v cargo >/dev/null 2>&1 || { echo "[FAIL] cargo missing" >&2; exit 2; }

cargo build --release --manifest-path "$GW/Cargo.toml"
export STCP_MQTT_GATEWAY_BIN="$GW/target/release/stcp-v2-mqtt-gateway"

RESULT_ROOT="${STCP_MQTT_ROBOT_RESULTS:-$TEST/results}"
RUN_ID="$(date +%Y%m%d-%H%M%S)"
RUN_DIR="$RESULT_ROOT/$RUN_ID"
mkdir -p "$RUN_DIR"

cd "$TEST"
rm -f mqtt-backend.log mqtt-gateway.log
set +e
robot --outputdir "$RUN_DIR" mqtt.robot
rc=$?
set -e
cp -f mqtt-backend.log mqtt-gateway.log "$RUN_DIR/" 2>/dev/null || true
ln -sfn "$RUN_DIR" "$RESULT_ROOT/latest"
(( rc == 0 )) && echo "[OK] MQTT/STCP application Robot PASS" || echo "[FAIL] MQTT/STCP application Robot rc=$rc" >&2
exit "$rc"
