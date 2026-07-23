#!/usr/bin/env bash
set -Eeuo pipefail
SOURCE_DIR="${1:-generated/raspberry-pi}"
TARGET="${WEB_DEPLOY_TARGET:-www-data@fuji:~/html/public/stcp.fi/benchmarks/raspberry-pi/}"
SSH_OPTS="${SSH_OPTS:--o StrictHostKeyChecking=accept-new}"
[[ -f "$SOURCE_DIR/index.html" && -f "$SOURCE_DIR/tcp/index.html" && -f "$SOURCE_DIR/udp/index.html" ]] || { echo "[FAIL] Generated TCP/UDP site incomplete: $SOURCE_DIR" >&2; exit 1; }
[[ -f "$SOURCE_DIR/manifest.json" ]] || { echo "[FAIL] Missing root manifest" >&2; exit 1; }
python3 - "$SOURCE_DIR" <<'PY'
import hashlib,json,sys
from pathlib import Path
r=Path(sys.argv[1]); m=json.loads((r/'manifest.json').read_text())
for x in m['files']:
 p=r/x['path']
 if not p.is_file() or hashlib.sha256(p.read_bytes()).hexdigest()!=x['sha256']: raise SystemExit(f"Checksum mismatch: {x['path']}")
print(f"[ OK ] {len(m['files'])} files verified")
PY
HOST="${TARGET%%:*}"; PATH_REMOTE="${TARGET#*:}"
if [[ "$PATH_REMOTE" == ~/* ]]; then HOME_REMOTE="$(ssh $SSH_OPTS "$HOST" 'printf %s "$HOME"')"; PATH_REMOTE="$HOME_REMOTE/${PATH_REMOTE#~/}"; fi
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"; STAGING="${PATH_REMOTE%/}.staging-$STAMP"; BACKUP="${PATH_REMOTE%/}.previous"
# shellcheck disable=SC2086
ssh $SSH_OPTS "$HOST" "rm -rf '$STAGING' && mkdir -p '$STAGING'"
if command -v rsync >/dev/null 2>&1; then
  # shellcheck disable=SC2086
  rsync -a --delete -e "ssh $SSH_OPTS" "$SOURCE_DIR/" "$HOST:$STAGING/"
else
  # shellcheck disable=SC2086
  scp -r $SSH_OPTS "$SOURCE_DIR/." "$HOST:$STAGING/"
fi
# shellcheck disable=SC2086
ssh $SSH_OPTS "$HOST" "set -e; rm -rf '$BACKUP'; if [ -d '$PATH_REMOTE' ]; then mv '$PATH_REMOTE' '$BACKUP'; fi; mv '$STAGING' '$PATH_REMOTE'"
printf '[ OK ] Published TCP + UDP dashboards to %s\n' "$TARGET"
