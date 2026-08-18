#!/usr/bin/env bash
set -euo pipefail

APP_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$APP_DIR"

exec python3 scripts/run-infra-benchmarks.py \
  --device /dev/ttyACM0 \
  --host 192.168.1.20 \
  --port 19000 \
  --transports tcp,stcp \
  --payloads 4096,8192,16384 \
  --directions full \
  --total 1048576
