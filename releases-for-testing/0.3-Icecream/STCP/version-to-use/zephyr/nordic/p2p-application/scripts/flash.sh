#!/usr/bin/env bash
set -Eeuo pipefail
APP_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd -P)"
BUILD="${STCP_P2P_BUILD_DIR:-$APP_ROOT/build}"
[[ -d "$BUILD" ]] || { echo "[FAIL] Build directory missing: $BUILD" >&2; exit 2; }
exec west flash -d "$BUILD" "$@"
