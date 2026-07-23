# STCP benchmark server accept fix v3

The STCP server no longer calls `libc.accept()` through `ctypes`.

On Raspberry Pi the TCP carrier connection reached the listener (`Recv-Q=1`),
but AF_STCP's protocol `accept` callback was not entered. The server now uses
CPython's native `socket._accept()` path, which returns the accepted file
descriptor without trying to decode an AF_STCP peer address.

Additional diagnostics:

- `benchmark server: waiting in accept ...`
- `benchmark server: accepted ... fd=N`
- accept errors are logged and retried instead of terminating the server
- TLS handshake failures are logged

Restart all benchmark servers after deploying the updated script.
