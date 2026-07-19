#!/bin/bash
set -e
echo "=== Build ==="
make LLVM=1 module
echo "=== Basic ==="
make test-basic
echo "=== Large ==="
make test-large
echo "=== Stress (20s) ==="
timeout 20s ./testing/stcp-stress || true
echo "Done."
