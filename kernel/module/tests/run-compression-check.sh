#!/bin/sh
set -eu
cd "$(dirname "$0")"
test_bin=$(mktemp /tmp/stcp-v7-compression.XXXXXX)
trap 'rm -f "$test_bin"' EXIT HUP INT TERM
rustc --edition=2024 -O -A dead_code compression_compare.rs -o "$test_bin"
"$test_bin"
