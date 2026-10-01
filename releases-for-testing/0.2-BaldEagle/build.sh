#!/usr/bin/env bash
set -euo pipefail
HERE="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
cargo build --release --manifest-path "$HERE/mini-sdk/Cargo.toml"
echo "[OK] $HERE/mini-sdk/target/release/stcp-echo-server"
echo "[OK] $HERE/mini-sdk/target/release/stcp-echo-client"
