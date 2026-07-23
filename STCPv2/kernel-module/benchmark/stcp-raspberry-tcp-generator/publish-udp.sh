#!/usr/bin/env bash
set -Eeuo pipefail

SOURCE_DIR="${1:-generated/raspberry-tcp}"
TARGET_UDP="${WEB_DEPLOY_TARGET_UDP:-www-data@fuji:~/html/public/stcp.fi/benchmarks/raspberry/udp/}"
SSH_OPTS="${SSH_OPTS:--o StrictHostKeyChecking=accept-new}"

[[ -f "$SOURCE_DIR/index.html" ]] || { echo "[FAIL] Missing $SOURCE_DIR/index.html" >&2; exit 1; }
[[ -f "$SOURCE_DIR/manifest.json" ]] || { echo "[FAIL] Missing manifest.json" >&2; exit 1; }

REMOTE_HOST="${TARGET_UDP%%:*}"
REMOTE_PATH="${TARGET_UDP#*:}"
if [[ "$REMOTE_PATH" == "~/"* ]]; then
  # Resolve the remote account home so atomic rename commands use an absolute path.
  # shellcheck disable=SC2086
  REMOTE_HOME="$(ssh $SSH_OPTS "$REMOTE_HOST" 'printf %s "$HOME"')"
  REMOTE_PATH="$REMOTE_HOME/${REMOTE_PATH#~/}"
fi
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
STAGING="${REMOTE_PATH%/}.staging-$STAMP"
BACKUP="${REMOTE_PATH%/}.previous"

printf '[INFO] Validating local manifest\n'
python3 - "$SOURCE_DIR" <<'PY'
import hashlib,json,sys
from pathlib import Path
root=Path(sys.argv[1]); manifest=json.loads((root/'manifest.json').read_text())
for item in manifest['files']:
    p=root/item['path']; h=hashlib.sha256(p.read_bytes()).hexdigest()
    if h != item['sha256']: raise SystemExit(f"Checksum mismatch: {item['path']}")
print(f"[ OK ] {len(manifest['files'])} files verified")
PY

# shellcheck disable=SC2086
ssh $SSH_OPTS "$REMOTE_HOST" "rm -rf '$STAGING' && mkdir -p '$STAGING'"
if command -v rsync >/dev/null 2>&1; then
  # shellcheck disable=SC2086
  rsync -a --delete -e "ssh $SSH_OPTS" "$SOURCE_DIR/" "$REMOTE_HOST:$STAGING/"
else
  # shellcheck disable=SC2086
  scp -r $SSH_OPTS "$SOURCE_DIR/." "$REMOTE_HOST:$STAGING/"
fi

# Atomic directory switch. The old deployment remains as .previous.
# shellcheck disable=SC2086
ssh $SSH_OPTS "$REMOTE_HOST" "set -e; rm -rf '$BACKUP'; if [ -d '$REMOTE_PATH' ]; then mv '$REMOTE_PATH' '$BACKUP'; fi; mv '$STAGING' '$REMOTE_PATH'"
printf '[ OK ] Published to %s\n' "$TARGET_UDP"
