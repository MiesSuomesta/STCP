#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
command -v cargo >/dev/null || { echo '[FAIL] cargo missing'; exit 1; }
command -v rustc >/dev/null || { echo '[FAIL] rustc missing'; exit 1; }
rustup run nightly rustc --version >/dev/null
[[ -f "$ROOT/rust/src/session.rs" ]]
grep -q 'External carrier mode' "$ROOT/rust/src/session.rs"
grep -q 'CONFIG_STCP_RUST_CORE' "$ROOT/CMakeLists.txt"
grep -q 'stcp_rust_start_handshake' "$ROOT/src/stcp_socket_offload.c"
echo '[OK] Rust/Zephyr integration files are present'
