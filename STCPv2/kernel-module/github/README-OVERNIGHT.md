# STCP extended overnight benchmark

This package adds a self-hosted GitHub Actions workflow for the extended
Raspberry Pi benchmark matrix.

## Matrix

Clients:

```text
1 2 4 8 16 32 64 128
```

Payloads:

```text
1024
65536
131072
262144
1048576
2097152
4194304
8388608
16777216
```

These correspond to 1 KiB, 64 KiB, 128 KiB, 256 KiB, 1 MiB, 2 MiB,
4 MiB, 8 MiB and 16 MiB. The requested 2048 KiB and 2 MiB are the same
payload, so the value is included only once.

Pipelines:

```text
1 4 8
```

The matrix contains 216 case groups for each carrier and 432 groups across
TCP and UDP. Each group compares three transports. At 20 seconds per
transport, the theoretical minimum runtime is about 7.2 hours before
deployment, perf and reporting overhead.

## Installation

Copy the files into the repository:

```text
.github/workflows/stcp-overnight-benchmark.yml
kernel-module/benchmark/run-overnight-matrix.sh
```

Make the runner executable:

```bash
chmod +x kernel-module/benchmark/run-overnight-matrix.sh
```

The GitHub self-hosted runner needs these labels:

```text
self-hosted
linux
x64
stcp-builder
```

It also needs passwordless SSH access to `pi@192.168.1.199`, access to
`perf`, and the existing STCP build/benchmark environment.

## Starting the workflow

The workflow runs nightly at 22:00 UTC and can also be launched from:

```text
GitHub → Actions → STCP extended overnight benchmark → Run workflow
```

Manual inputs can change the duration and pipeline list. A successful manual
run can optionally create and push an annotated golden tag.

Scheduled runs upload the result archive and logs as a GitHub Actions
artifact but do not create tags.

## Local test

From `kernel-module`:

```bash
DURATION=5 \
CLIENTS_LIST="1 2" \
PAYLOADS="1024 65536" \
PIPELINES="1" \
bash benchmark/run-overnight-matrix.sh
```
