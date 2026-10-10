#!/usr/bin/env bash
set -euo pipefail
ROOT=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
python3 "$ROOT/apply-pool16.py"
cd "$ROOT/application"
bash scripts/build-ethernet.sh
python3 "$ROOT/check-pool16.py"
bash scripts/flash-ethernet.sh
