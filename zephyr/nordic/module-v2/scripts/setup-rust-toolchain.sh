#!/usr/bin/env bash
set -euo pipefail
if ! command -v rustup >/dev/null 2>&1; then
  echo "[FAIL] rustup is not installed. Install from https://rustup.rs" >&2
  exit 1
fi
rustup toolchain install nightly --component rust-src
rustup target add thumbv8m.main-none-eabi --toolchain nightly || true
rustup component add rust-src --toolchain nightly
rustc +nightly --version
cargo +nightly --version
echo "[OK] Rust toolchain ready for STCP Zephyr integration"
