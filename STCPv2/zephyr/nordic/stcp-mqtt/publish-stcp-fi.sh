#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
exec bash benchmark/publish-stcp-fi.sh "$@"
