#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
python3 "$ROOT/generate-site.py" "$@"
python3 "$ROOT/sync-common-menu.py"
