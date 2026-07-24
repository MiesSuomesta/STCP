#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${PROJECT_ROOT:-$(cd "$SCRIPT_DIR/../.." && pwd)}"
WEB_DIR="${WEB_DIR:-$PROJECT_ROOT/web}"

[[ -x "$WEB_DIR/publish-latest.sh" ]] || {
  echo "[FAIL] Missing executable: $WEB_DIR/publish-latest.sh" >&2
  exit 1
}

 PROJECT_ROOT="$PROJECT_ROOT" \
 BENCHMARK_DIR="$SCRIPT_DIR" \
 WEB_DIR="$WEB_DIR" \
 PUBLISH_TARGET="${PUBLISH_TARGET:-www-data@fuji:/var/www/html/public/stcp.fi/}" \
	"$WEB_DIR/publish-latest.sh"
