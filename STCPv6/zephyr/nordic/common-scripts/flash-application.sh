#!/usr/bin/env bash
set -Eeuo pipefail

die() { printf '[FAIL] %s\n' "$*" >&2; exit 1; }
(( $# >= 1 )) || die "Usage: $0 {application|app-coap|app-mqtt|p2p-application} [west flash options]"
APP_NAME="$1"; shift
case "$APP_NAME" in application|app-coap|app-mqtt|p2p-application) ;; *) die "Unknown application: $APP_NAME" ;; esac
GIT_TOP="$(git rev-parse --show-toplevel)" || die "Run inside the STCP Git repository"
if [[ -d "$GIT_TOP/stcp/common-scripts" ]]; then
    STCP_ZEPHYR_ROOT="$GIT_TOP/stcp"
    WEST_ROOT="$GIT_TOP"
elif [[ -e "$GIT_TOP/STCP/version-to-use" || -e "$GIT_TOP/version-to-use" ]]; then
    if [[ -e "$GIT_TOP/STCP/version-to-use" ]]; then
        VERSION="$(readlink -f "$GIT_TOP/STCP/version-to-use")"
        PROJECT_ROOT="$GIT_TOP"
    else
        VERSION="$(readlink -f "$GIT_TOP/version-to-use")"
        PROJECT_ROOT="$GIT_TOP/.."
    fi
    STCP_ZEPHYR_ROOT="$VERSION/zephyr/nordic"
    WEST_ROOT="${ZEPHYR_WORKSPACE:-$PROJECT_ROOT/zephyr-stcp}"
else
    die "Cannot find stcp/ or STCP/version-to-use under $GIT_TOP"
fi
BUILD_DIR="${STCP_V2_BUILD_DIR:-$STCP_ZEPHYR_ROOT/$APP_NAME/build-ethernet}"
PYTHON="${WEST_PYTHON:-$WEST_ROOT/.venv/bin/python}"
[[ -d "$BUILD_DIR" ]] || die "Build missing: $BUILD_DIR (run build-ethernet.sh first)"
[[ -x "$PYTHON" ]] || die "Missing west Python: $PYTHON"
unset PYTHONHOME PYTHONPATH
cd "$WEST_ROOT"
exec "$PYTHON" -m west flash -d "$BUILD_DIR" "$@"
