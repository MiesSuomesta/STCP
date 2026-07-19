# STCPv2 V6.3 synchronous connect handshake fix

The Python stress client performs a blocking `connect()` and switches the file
descriptor to nonblocking mode immediately afterwards.  The old kernel path
only started the STCP handshake and returned success before the socket reached
`Ready`.  The first `send()` therefore raced the handshake and commonly timed
out through `poll(POLLOUT)`.

Changes:

- blocking `stcp_connect()` now waits on `recv_wq` until
  `stcp_rust_is_connected()` reports the `Ready` state;
- nonblocking connect returns `-EINPROGRESS` and leaves the socket in
  `SS_CONNECTING`;
- carrier receive wakes `recv_wq`, so handshake progress re-evaluates the wait
  condition without busy polling.

This preserves the V6.2 kthread lifetime fix and the safe ByteQueue/RX fast-path
optimizations.
