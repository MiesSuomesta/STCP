#!/usr/bin/env bash
set -euo pipefail
APP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
NORDIC_DIR="$(cd "$APP_DIR/.." && pwd)"
NCS_DIR="$NORDIC_DIR/ncs-3.3.0"
PYTHON="$NCS_DIR/.venv/bin/python"
unset PYTHONHOME PYTHONPATH
exec "$PYTHON" -m west flash -d "$APP_DIR/build-ethernet"
