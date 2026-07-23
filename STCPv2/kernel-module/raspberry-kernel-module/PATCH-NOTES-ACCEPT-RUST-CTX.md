# TCP accept carrier context fix

The accepted TCP carrier was created before the accepted Rust child context.
Only `owner` was wired afterward, leaving `carrier->rust_ctx == NULL`. The RX
thread therefore passed NULL to `stcp_rust_carrier_receive_from()`, which
returned `-EINVAL`, and the accept handshake timed out after 30 seconds.

Fix:

- add `stcp_carrier_set_rust_ctx()`
- wire the accepted child context before starting the receiver thread
- snapshot and validate the context in the RX dispatch path
