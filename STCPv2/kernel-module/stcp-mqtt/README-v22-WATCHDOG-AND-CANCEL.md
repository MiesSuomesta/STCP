# v22: watchdog, background worker and cancellation

The benchmark shell no longer executes network transfers in the shell thread.
Upload, download, full-duplex and all-tests runs start in a dedicated worker
thread, leaving UART shell commands responsive.

## Commands

```text
stcp bench upload
stcp bench download
stcp bench full
stcp bench all
stcp bench status
stcp bench stop
```

## Inactivity watchdog

`stcp config timeout <milliseconds>` is an inactivity timeout, not a maximum
benchmark duration. Every successful send or receive refreshes it. If no bytes
move before the timeout expires, delayed work calls `shutdown()` on the active
socket and the worker exits with `-ETIMEDOUT`.

The default remains 30000 ms from `CONFIG_ECHO_SOCKET_TIMEOUT_MS`. For the slow
LTE-M link, 60000 or 120000 ms may be more appropriate:

```text
stcp config timeout 60000
```

`stcp bench stop` requests cancellation and shuts down the active socket. The
worker remains the sole owner that closes the descriptor, avoiding close/reuse
races.
