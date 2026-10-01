#!/usr/bin/env bash
set -Eeuo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
VENV="${STCP_ROBOT_VENV:-$HERE/.venv}"
python3 -m venv "$VENV"
"$VENV/bin/python" -m pip install -U pip
"$VENV/bin/python" -m pip install -r "$HERE/requirements.txt"
echo "[OK] Activate with: source '$VENV/bin/activate'"
