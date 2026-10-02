# STCPv2 Zephyr Real-World Soak Benchmark

This is a host-side orchestrator for the existing Zephyr STCPv2 benchmark shell. It does **not** require firmware changes.

It drives the existing commands:

- `stcp config transport stcp`
- `stcp config host <host>`
- `stcp config port <port>`
- `stcp config chunk <bytes>`
- `stcp config total <bytes>`
- `stcp bench upload`
- `stcp bench download`
- `stcp bench full`

and parses the current `STCP_BENCH_JSON_BEGIN / STCP_BENCH_JSON_PART / STCP_BENCH_JSON_END` output.

## What it tests

The workload intentionally avoids a single synthetic steady-state case. One long run contains:

- upload, download and full-duplex traffic
- randomized message/chunk sizes: 256 .. 16384 bytes by default
- randomized transfer sizes: 64 KiB .. 8 MiB by default
- full-duplex pressure bursts
- repeated short connections / handshake churn
- idle -> immediate traffic transitions
- randomized inter-request gaps
- optional external link/fault injection and recovery
- crash signature detection from the Zephyr console
- timeout detection
- per-case JSONL results plus raw console capture

The benchmark exits non-zero if any benchmark case fails, times out or crashes. Use `--continue-after-failure` (default) to keep collecting evidence after failures. Use `--stop-on-failure` when debugging the first failure.

## Default ports

- upload: 19000
- download: 19001
- full duplex: 19002

These can all be overridden.

## Quick run

```bash
STCP_ZEPHYR_SERIAL=/dev/ttyACM0 \
STCP_SERVER_HOST=192.168.1.20 \
./run-realworld-soak.sh --duration 3600
```

Eight-hour default:

```bash
STCP_ZEPHYR_SERIAL=/dev/ttyACM0 ./run-realworld-soak.sh
```

Harder overnight run:

```bash
./run-realworld-soak.sh \
  --serial /dev/ttyACM0 \
  --host 192.168.1.20 \
  --duration 43200 \
  --max-total 16777216 \
  --case-timeout 120 \
  --reconnect-burst 25
```

## Fault injection

The orchestrator can invoke external commands. This is deliberately generic so the fault can be implemented with a managed switch, relay, remote shell, interface command, or whatever the lab setup supports.

Example where a Linux bridge/interface is used as the fault point:

```bash
./run-realworld-soak.sh \
  --fault-every 50 \
  --fault-down-seconds 5 \
  --fault-command 'ssh root@gateway ip link set eth1 down' \
  --fault-recover-command 'ssh root@gateway ip link set eth1 up'
```

Template variables `{seq}`, `{phase}` and `{direction}` are available inside the commands.

Do not use a fault command against the interface carrying your SSH control connection unless you have another recovery path.

## Output

Each run creates `stcp-realworld-YYYYmmdd-HHMMSS/` containing:

- `run.json` — exact test parameters and PRNG seed
- `results.jsonl` — one machine-readable record per benchmark case
- `summary.json` — aggregate pass/fail/timeout/crash and byte counters
- `zephyr-console.log` — complete console capture
- `events.log` — compact run timeline
- `fault-hooks.log` — external fault command output

A failed run can be replayed approximately with the same `--seed` and parameters.

## Suggested acceptance gates

For a meaningful "real-world stable" claim, use at least:

1. 8 hours with zero crash, zero timeout and zero benchmark error.
2. 12-24 hours with external link faults every 50-100 cases and successful post-fault traffic.
3. A separate 1-2 hour full-duplex-heavy run with `--max-total 16777216`.
4. Compare the first and last hour throughput/error distribution to catch degradation or leaks.
5. Keep Linux kernel/netconsole logs alongside this result directory when testing the Linux STCP peer.

The most important result is not peak throughput; it is **continued correct operation through connection churn, traffic shape changes, quiet periods, saturation and recoverable network faults**.

## Serial ownership guard

The orchestrator takes a non-blocking process lock and opens the Linux serial
port exclusively. Starting a second soak against the same Zephyr console exits
with code 3 instead of allowing two runs to interleave commands and results.
Use `pgrep -af stcp_realworld_soak.py` to locate a stale runner.
