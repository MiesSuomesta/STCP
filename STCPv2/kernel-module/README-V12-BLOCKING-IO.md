# V12 blocking modem I/O and full-duplex completion handshake

Changes:

- Benchmark sockets are returned to blocking mode after nonblocking connect.
- Upload uses blocking `send()` and no `MSG_DONTWAIT`/`POLLOUT` loop.
- Unsupported modem-offload `SO_SNDBUF`/`SO_RCVBUF` tuning was removed.
- Download receives arbitrary TCP stream fragments and validates each fragment.
- Python server receives streams with `recv_into()` instead of `recv_exact(chunk)`.
- Full-duplex now has an explicit client completion ACK before the server sends its final reply and closes the connection.
- This prevents the server-side `ConnectionResetError` caused by one endpoint closing while the other direction was still completing.
