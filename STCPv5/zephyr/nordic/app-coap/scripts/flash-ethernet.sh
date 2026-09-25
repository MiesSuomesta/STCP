#!/usr/bin/env bash
set -Eeuo pipefail
GIT_TOP="$(git rev-parse --show-toplevel)" || { echo '[FAIL] Run inside the STCP Git repository' >&2; exit 1; }
if [[ -d "$GIT_TOP/stcp/common-scripts" ]]; then
    COMMON="$GIT_TOP/stcp/common-scripts"
elif [[ -e "$GIT_TOP/STCP/version-to-use" ]]; then
    COMMON="$(readlink -f "$GIT_TOP/STCP/version-to-use")/zephyr/nordic/common-scripts"
elif [[ -e "$GIT_TOP/version-to-use" ]]; then
    COMMON="$(readlink -f "$GIT_TOP/version-to-use")/zephyr/nordic/common-scripts"
else
    echo "[FAIL] version-to-use missing under $GIT_TOP/STCP and $GIT_TOP" >&2
    exit 1
fi
[[ -f "$COMMON/flash-application.sh" ]] || { echo "[FAIL] Missing $COMMON/flash-application.sh" >&2; exit 1; }
exec bash "$COMMON/flash-application.sh" app-coap "$@"
