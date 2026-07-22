#!/usr/bin/env bash
set -Eeuo pipefail
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
for port in raspberry-kernel-module x86-kernel-module; do
  echo "== $port =="
  grep -n 'bool stcp_carrier_is_udp' "$ROOT/$port/include/stcp_carrier.h"
  grep -n 'stcp_rust_create_stream_accepted_child' "$ROOT/$port/include/stcp_rust_ffi.h"
  grep -n 'bool stcp_carrier_is_udp' "$ROOT/$port/src/stcp_ops.c"
  grep -n 'stcp_rust_create_stream_accepted_child' "$ROOT/$port/src/stcp_ops.c"
  grep -n 'bool stcp_carrier_is_udp' "$ROOT/$port/src/stcp_carrier.c"
done
grep -n 'pub extern "C" fn stcp_rust_create_stream_accepted_child' \
  "$ROOT/common-rust/src/ffi.rs"
echo '[ OK ] headers, local declarations and Rust export are present'
