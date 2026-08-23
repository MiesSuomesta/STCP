#!/usr/bin/env bash
set -euo pipefail
APP_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$APP_DIR/../../.." && pwd)"
exec bash "$REPO_ROOT/tests/benchmark/zephyr/publish-stcp-fi.sh" "$@"
