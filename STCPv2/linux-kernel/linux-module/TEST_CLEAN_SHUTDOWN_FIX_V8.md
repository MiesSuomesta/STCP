# V8: stress-test clean shutdown and resource release

Changes:

- Added a thread-safe registry for every STCP listener, accepted socket and client socket.
- `StopState.stop()` closes all registered file descriptors immediately, waking blocked `poll`, `accept`, `send` and `recv` calls.
- All worker and server threads are non-daemon and are joined before the process exits.
- Added a bounded shutdown deadline and explicit failure if a thread still remains alive.
- Added unregister-before-close cleanup to avoid stale descriptors in the registry.
- Added robust `SIGINT` / `SIGTERM` cleanup to `run_stress_suite.sh`.
- The shell runner terminates its active Python child, waits for graceful shutdown, and uses SIGKILL only as a last resort.
- Corrected the throughput test heading to 25 seconds.
