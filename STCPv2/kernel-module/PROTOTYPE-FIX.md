# Accept/handshake prototype fix v2

This package includes the declarations required by both kernel ports:

- `stcp_carrier_is_udp()` in `include/stcp_carrier.h`
- `stcp_rust_create_stream_accepted_child()` in `include/stcp_rust_ffi.h`

Both `stcp_ops.c` files include these headers before use, and both
`stcp_carrier.c` files include their own public carrier header before the
function definitions. Run `./verify-accept-fix.sh` before building.
