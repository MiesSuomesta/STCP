#!/usr/bin/env bash
set -Eeuo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
python3 -m venv "$HERE/.venv"
"$HERE/.venv/bin/python" -m pip install --upgrade pip
"$HERE/.venv/bin/python" -m pip install -r "$HERE/requirements.txt"
echo "[OK] $HERE/.venv"
