#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
binary=$(mktemp /tmp/stcp-pool.XXXXXX)
trap 'rm -f "$binary"' EXIT
cc -std=c11 -Wall -Wextra -Werror -pthread -fsanitize=address,undefined -g -DCONFIG_STCP_BUFFER_POOL_FRAMES=32 -DCONFIG_STCP_BUFFER_POOL_LARGE=2 -DCONFIG_STCP_LOG_LEVEL=3 -Ivalidation/shim -Iinclude src/stcp_buffer_pool.c validation/pool-test.c -o "$binary"
ASAN_OPTIONS=detect_leaks=0 "$binary"
