# Carrier lifecycle fix applied

The crash in `kthread_stop()` was caused by mismatched lifetimes between a UDP
listener/root carrier, its receiver task, accepted UDP child carriers, and the
listener Rust context.

Changes:

- UDP children no longer copy the listener's raw `struct socket *`.
- UDP sends resolve the socket through the refcounted parent/root carrier.
- Root shutdown is two-phase:
  1. stop/join RX and invalidate callbacks immediately when the listener closes;
  2. free the root object/socket only after the last UDP child drops its ref.
- `lifecycle_lock` serializes root socket send vs shutdown.
- Listener teardown cannot resurrect a zero root refcount during accept.
- `stcp_release()` detaches carrier/Rust pointers with `xchg()` exactly once.
- Receiver shutdown happens before the listener Rust context is freed.

This prioritizes memory safety. Existing accepted UDP children return
`-ESHUTDOWN` after their listener/root is closed instead of using a stale socket.
