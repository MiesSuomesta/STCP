#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd -P)"
TEST="$ROOT/application/testing/robot-v2"

: "${STCP_ZEPHYR_SERIAL:=/dev/ttyACM0}"
: "${STCP_ZEPHYR_SERVER_HOST:=192.168.1.20}"
: "${STCP_ZEPHYR_SERVER_PORT:=19000}"

make -C "$TEST/server"
export STCP_ZEPHYR_SERVER_BIN="$TEST/server/stcp-v2-bench-server"
export STCP_ZEPHYR_SERIAL STCP_ZEPHYR_SERVER_HOST STCP_ZEPHYR_SERVER_PORT

cd "$TEST"
exec robot --outputdir results zephyr-v2.robot
