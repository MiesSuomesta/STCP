# STCP V6.2 – receiver kthread lifetime fix

## Crash

The module crashed in `kthread_stop()` because the TCP receiver thread could
return by itself after EOF, shutdown, reset, or `-EPIPE`. The carrier retained
`carrier->receiver`, so later teardown passed a stale task pointer to
`kthread_stop()`.

## Fix

The receiver thread now has one authoritative lifetime owner:

- it may observe a terminal TCP receive condition;
- it parks instead of returning;
- it returns only after `kthread_should_stop()` becomes true;
- `stcp_carrier_stop_root()` remains the only path that stops and joins it;
- `carrier->receiver` is detached under `lifecycle_lock` before the join.

This prevents a completed kthread from invalidating the pointer before socket
release.
