#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"
APP_ROOT="${STCP_V2_APP_ROOT:-$(cd "$SCRIPT_DIR/.." && pwd -P)}"
BUILD="${STCP_V2_BUILD_DIR:-$APP_ROOT/build-v2-clean}"

[[ -d "$BUILD" ]] || {
    echo "[FAIL] Build directory missing: $BUILD" >&2
    echo "[INFO] Application root: $APP_ROOT" >&2
    echo "[INFO] Run: $APP_ROOT/scripts/build-v2-clean.sh" >&2
    exit 2
}

exec west flash -d "$BUILD" "$@"
