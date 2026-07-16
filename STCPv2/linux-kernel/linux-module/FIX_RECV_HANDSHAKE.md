# Fix: recv during asynchronous handshake

The accepted server-side child socket can be returned to userspace before the
carrier handshake reaches `SocketState::Ready`.  Previously `session::recv()`
called `fill_application_buffer()`, which treated `Handshake` as an invalid
state and returned `-EINVAL`.

The fix makes `session::recv()` return `StcpError::Again` while the context is
in `Handshake`.  The C `stcp_recvmsg()` wrapper already handles this correctly:
it waits interruptibly on `recv_wq` and retries when the carrier receiver wakes
the socket.

This also prevents the server test process from exiting early, which previously
closed the carrier and caused the client-side send to fail with `-ENOTCONN`.
