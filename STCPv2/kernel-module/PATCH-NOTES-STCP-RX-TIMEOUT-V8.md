# STCP TCP receive-timeout fix (debug-v8)

## Root cause

The per-socket parser used a boolean try-lock. If carrier RX appended bytes while
another parser invocation was active, the second invocation returned. A narrow
race existed when the active parser had already observed an empty queue but had
not yet cleared the boolean: the newly appended complete frame remained queued
without another parsing pass, so userspace recv() slept until its timeout.

## Fix

- Replace the boolean with a three-state atomic parser state:
  - 0: idle
  - 1: active
  - 2: active with pending work
- Concurrent RX marks pending work instead of silently returning.
- The active parser repeats a full pass before releasing ownership whenever
  pending work was observed.
- The handshake parser peeks before removal and leaves the first non-handshake
  frame in the queue. This handles HandshakeDone and DATA arriving in one TCP
  read.
- Debug marker bumped to stcp-debug-v8.

## Regression test

```bash
RPI_HOST=192.168.1.199 \
RPI_SSH=pi@192.168.1.199 \
RPI_BENCHMARK_DIR=/home/pi/benchmark \
CLIENTS=2 PAYLOAD=64 PIPELINE=1 DURATION=30 \
./benchmark/run-timeout-regression.sh 10
```

Expected result: 10 runs, zero `STCP receive timed out` errors.
