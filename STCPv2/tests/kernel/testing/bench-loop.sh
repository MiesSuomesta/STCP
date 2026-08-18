#!/bin/bash
set -e
for i in 1 2 3 4 5; do
  echo "Run $i"
  SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
  /usr/bin/time -f "elapsed=%e" "$SCRIPT_DIR/stcp_large_test" client
done
