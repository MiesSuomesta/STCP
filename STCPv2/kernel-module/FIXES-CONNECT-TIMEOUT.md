# STCP connect/matrix hang fixes

This package fixes two independent failure modes seen in the benchmark matrix.

## Kernel module

- `connect_timeout_ms` is now a module parameter, defaulting to 30000 ms.
- Both blocking `connect()` and server-side `accept()` handshake waits use it.
- The carrier RX thread now wakes the socket receive waitqueue after every
  successfully received carrier block. Rust still performs precise Ready/data
  wakeups; the C wake is a safe redundant wake that prevents a lost handshake
  wake from leaving connect asleep until timeout.

Runtime adjustment:

```bash
sudo sh -c 'echo 30000 > /sys/module/stcp/parameters/connect_timeout_ms'
```

## Benchmark client

- If one client fails to connect, it aborts the startup barrier so peer clients
  do not wait forever.
- Barrier waits have a timeout.
- Benchmark duration begins after all clients have connected.

## Matrix runner

- Every case runs under GNU `timeout`.
- Defaults:
  - `CONNECT_TIMEOUT=30`
  - `CASE_GRACE_SECONDS=30`
  - `CASE_TIMEOUT=DURATION + CONNECT_TIMEOUT + CASE_GRACE_SECONDS`
- A timed-out case gets a JSON failure result instead of hanging the full run.

## Server startup

`start-servers.sh` waits for the READY log line from TCP, TLS and STCP servers
and detects a server process that exits before becoming ready.
