# STCP Benchmark v2

Benchmark v2 keeps every run immutable and separates measured case data from
validation and publication approval. A failed run remains useful evidence but
cannot be published.

## First run

Start the TCP, TLS 1.3 and STCP servers on the target using the existing
`../raspberrypi/start-servers.sh`. Then run from the controller:

```bash
RUN_DIR="$PWD/results/run-$(date -u +%Y%m%dT%H%M%SZ)" \
BENCHMARK_HOST=raspi \
RESTART_SERVERS='ssh pi@raspi /home/pi/benchmark/start-servers.sh' \
TARGET_SSH=pi@raspi \
./run-all.sh
```

Production runs must pin an explicit `CASES=/path/to/cases.tsv`. `RUN_DIR` is
required; no script guesses a `latest` directory. Each failed case is retried
up to five times and servers are restarted before retries.

Successful runs contain:

- `run.json`: source and controller provenance
- `cases.tsv`: exact matrix used for the run
- `attempts/`: original benchmark JSON from every attempt
- `cases/`: normalized accepted results
- `logs/`: per-attempt logs
- `environment/`: controller and optional target hardware/software snapshots
- `validation.json`: fail-closed suite verdict
- `report/index.html`: deterministic comparison report
- `SHA256SUMS` and `publish-approved.json`: publication capability

Results are imported into SQLite only after complete validation. The database
is intentionally derived data: immutable run directories remain the source of
truth and can later be imported into PostgreSQL or analysed by AI.

Publish only through the gate:

```bash
./deploy-atomic.sh /absolute/path/to/run lja@fuji:/var/www/html/public/stcp.fi/benchmarks/current
```

The current matrix is a small smoke/IoT/throughput baseline. Linux/RPi,
Zephyr, UDP, CoAP, MQTT, P2P, fault injection, wire-byte capture, perf and
energy collectors are the next adapters; they will write the same case schema.
