# Carrier/handshake ordering fix

- `stcp_connect()` connects the kernel carrier first, creates the Rust endpoint, then starts the handshake.
- Rust `session::connect()` no longer attempts a symmetric handshake before the accepted carrier exists.
- `stcp_accept()` attaches the accepted carrier and owner before starting the server-side handshake.
- Carrier receive automatically advances handshake frames, allowing both endpoints to reach `Ready` asynchronously.
- `send`, `recv`, polling helpers also advance an in-progress handshake.
- `stcp_sockaddr()` now uses `sockaddr_storage` and passes `sizeof(struct sockaddr_in)` to kernel socket calls.
- Added missing local `ret` declarations and carrier null checks in bind/listen.

Build on the kernel host with:

    make clean
    make all test
