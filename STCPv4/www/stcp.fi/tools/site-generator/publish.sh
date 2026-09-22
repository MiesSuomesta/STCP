#!/bin/bash
set -euo pipefail
STAGE="${1:?usage: publish.sh STAGE [WEBROOT]}"
WEBROOT="${2:-/nfs/fuji-www/html/public}"
NEW="$WEBROOT/uus-stcp.fi"
LIVE="$WEBROOT/stcp.fi"
OLD="$WEBROOT/old-stcp.fi"

test -f "$STAGE/index.html"
grep -q '<title>STCP</title>' "$STAGE/index.html"

rm -rf -- "$NEW"
mkdir -p -- "$NEW"
cp -a -- "$STAGE"/. "$NEW"/

test -f "$NEW/index.html"
grep -q '<title>STCP</title>' "$NEW/index.html"

rm -rf -- "$OLD"
if [[ -e "$LIVE" ]]; then mv -- "$LIVE" "$OLD"; fi
if ! mv -- "$NEW" "$LIVE"; then
    [[ -e "$OLD" && ! -e "$LIVE" ]] && mv -- "$OLD" "$LIVE"
    exit 1
fi
echo "[OK] Published $LIVE"
