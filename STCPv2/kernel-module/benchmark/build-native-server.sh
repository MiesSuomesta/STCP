#!/usr/bin/env bash
set -euo pipefail
cc -O2 -Wall -Wextra -pthread -o stcp_native_server stcp_native_server.c
echo "built: $PWD/stcp_native_server"
