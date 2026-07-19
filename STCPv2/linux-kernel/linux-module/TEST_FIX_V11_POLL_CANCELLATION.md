# Test fix V11: deterministic poll cancellation

The functional smoke test could complete the echo exchange but still fail at
shutdown because closing an STCP fd from another Python thread did not wake a
blocking poll() immediately.

Changes:
- NativeStcpSocket._wait() polls in at most 100 ms slices.
- Every slice re-checks whether close() replaced the fd or marked the socket closed.
- Cleanup waits up to two seconds after closing all tracked sockets.
- A still-running server thread is reported explicitly only after cancellation fails.

This keeps normal operation timeout semantics while making Ctrl+C and per-case
shutdown deterministic even when the custom kernel protocol does not wake poll
on cross-thread close.
