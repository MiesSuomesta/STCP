# STCP benchmark-v2 — phase 1

Clean benchmark harness for the current golden Zephyr application.

Phase 1 deliberately does **not** modify firmware sources. It uses the existing
`stcp config ...` and `stcp bench upload|download|full` commands and adds a new,
repeatable host-side measurement harness.

Covered transports:

- native TCP
- STCP-TCP (Linux AF_STCP=45, protocol 253)

Covered directions:

- upload
- download
- full duplex

Each case supports warmups + repeated measured runs. The runner stores raw
serial logs, host server logs, machine-readable JSON/CSV and a median/min/max
summary. Host benchmark-server CPU usage is sampled from `/proc/<pid>/stat`.

## Quick start

From `zephyr/nordic/application`:

```bash
cd testing/benchmark-v2
./run-benchmark.sh \
  --serial /dev/ttyACM0 \
  --host 192.168.1.20 \
  --total 8388608 \
  --chunk 8192 \
  --warmups 1 \
  --runs 5
```

Defaults are intentionally conservative and can be shown with:

```bash
./run-benchmark.sh --help
```

Results are written below `testing/benchmark-v2/results/<timestamp>/`.

## Benchmark policy

Keep this phase unchanged as the pre-optimization measuring stick. Run it once
before optimization, save/tag the result, then rerun the exact same command
and workload after every optimization phase.

The runner always tears down the host benchmark server on normal exit, error,
Ctrl-C and TERM.

## Why only TCP and STCP-TCP in phase 1?

The golden application already exposes these two stream transports through the
same BEN2 benchmark core. Adding UDP/STCP-UDP requires a firmware-side BEN2
datagram implementation, so that is intentionally a separate phase rather than
mixing a new transport implementation into the measurement baseline.
