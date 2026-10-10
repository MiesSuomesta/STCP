#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
binary=$(mktemp /tmp/ben2-matrix.XXXXXX)
trap 'rm -f "$binary"' EXIT
cc -O2 -std=gnu11 -Wall -Wextra -Werror -pthread -DHOST_SIMULATE_STCP=1 -Ivalidation/shim -Isrc src/echo_benchmark.c validation/matrix-main.c -o "$binary"
python3 validation/check-matrix.py "$binary"
