#!/usr/bin/env bash
set -euo pipefail
STCP_PROJECT_ROOT="${STCP_PROJECT_ROOT:-/srv/stcp-project}"
source "${STCP_SETTINGS:-$STCP_PROJECT_ROOT/settings.sh}"
cd "$STCP_ROOT"
echo "========== HOST BUILD =========="
bash scripts/build-all.sh host
echo "========== HOST INSTALL =========="
bash scripts/install-all.sh host
echo "[OK] Host build + install complete"
