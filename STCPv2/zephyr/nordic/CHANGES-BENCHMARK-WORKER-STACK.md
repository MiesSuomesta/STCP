# Dedicated benchmark worker

The interactive `stcp bench upload|download|full|all` commands no longer run
benchmark or Rust STCP socket/session creation in the `shell_uart` thread.

## Why

The Rust core needs substantially more temporary stack during AF_STCP socket
and session creation than the Zephyr shell thread provides. Running it directly
from the shell caused:

```
USAGE FAULT: Stack overflow
Current thread: shell_uart
```

## Implementation

- A persistent `stcp_bench` worker thread handles benchmark execution.
- Commands copy the current runtime configuration into a pending job and return.
- Only one benchmark may run at a time (`-EBUSY` otherwise).
- The worker emits the existing machine JSON and success/failure lines.
- Stack watermark information is printed when `CONFIG_THREAD_STACK_INFO=y`.

Ethernet defaults:

```
CONFIG_BENCH_WORKER_STACK_SIZE=8192
CONFIG_BENCH_WORKER_PRIORITY=5
CONFIG_THREAD_STACK_INFO=y
```

The Ethernet build also reduces `CONFIG_MAIN_STACK_SIZE` from 8192 to 4096, because benchmark work no longer runs on the main or shell thread. This offsets most of the dedicated worker stack RAM cost.
