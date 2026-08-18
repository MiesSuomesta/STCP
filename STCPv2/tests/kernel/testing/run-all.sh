#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
MODULE_ROOT="$REPO_ROOT/kernel/module"
TEST_ROOT="$REPO_ROOT/tests/kernel"
cd "$MODULE_ROOT"
echo "=== Build ==="
make LLVM=1 module
echo "=== Basic ==="
make test-basic
echo "=== Large ==="
make test-large
echo "=== Stress (20s) ==="
timeout 20s "$TEST_ROOT/stress/stcp-stress" || true
echo "Done."
