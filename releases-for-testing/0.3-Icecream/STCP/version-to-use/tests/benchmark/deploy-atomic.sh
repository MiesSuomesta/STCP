#!/usr/bin/env bash
set -euo pipefail
if (($# != 2)); then echo "usage: $0 RUN_DIR USER@HOST:/absolute/path" >&2; exit 2; fi
RUN_DIR=$(realpath "$1") TARGET=$2
[[ -f "$RUN_DIR/publish-approved.json" && -f "$RUN_DIR/SHA256SUMS" ]] || { echo "run is not publication-approved" >&2; exit 1; }
(cd "$RUN_DIR" && sha256sum -c SHA256SUMS)
REMOTE=${TARGET%%:*}; PATH_ON_HOST=${TARGET#*:}
[[ $PATH_ON_HOST == /* ]] || { echo "remote path must be absolute" >&2; exit 2; }
STAMP=$(date -u +%Y%m%dT%H%M%SZ)
STAGE="$PATH_ON_HOST.stage-$STAMP"
tar -C "$RUN_DIR/report" -cf - . | ssh "$REMOTE" "set -eu; mkdir -p '$STAGE'; tar -C '$STAGE' -xf -"
ssh "$REMOTE" "set -eu; test -f '$STAGE/index.html'; if test -e '$PATH_ON_HOST'; then mv '$PATH_ON_HOST' '$PATH_ON_HOST.previous-$STAMP'; fi; mv '$STAGE' '$PATH_ON_HOST'"
echo "published $RUN_DIR to $TARGET"
