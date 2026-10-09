#!/usr/bin/env bash
set -Eeuo pipefail

SERIAL_DEV="${STCP_ZEPHYR_SERIAL:-/dev/ttyACM0}"
BAUD="${STCP_ZEPHYR_BAUD:-115200}"
HOST="${STCP_SERVER_HOST:-192.168.1.20}"
DURATION="${STCP_SOAK_DURATION:-28800}"

HERE="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
VENV="${STCP_SOAK_VENV:-$HERE/.venv}"

if [[ ! -x "$VENV/bin/python" ]]; then
    python3 -m venv "$VENV"
    "$VENV/bin/python" -m pip install --upgrade pip
    "$VENV/bin/python" -m pip install -r "$HERE/requirements.txt"
fi

exec "$VENV/bin/python" "$HERE/stcp_realworld_soak.py" \
    --serial "$SERIAL_DEV" \
    --baud "$BAUD" \
    --host "$HOST" \
    --duration "$DURATION" \
    "$@"
