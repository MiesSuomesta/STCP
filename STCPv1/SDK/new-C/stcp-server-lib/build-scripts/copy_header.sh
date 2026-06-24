#!/bin/bash
set -e
TARGET_HEADER=$(find ../target/*/build -name 'stcp_server_cwrapper_lib.h' | head -n1)
echo "$TARGET_HEADER" 
