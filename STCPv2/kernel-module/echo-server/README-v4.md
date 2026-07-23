# Throughput benchmark v4

Start plain TCP server:

```bash
sudo python3 echo_server.py --no-tls --no-stcp --verbose
```

The Zephyr client runs these tests in order:

1. Upload: nRF -> server
2. Download: server -> nRF
3. Full duplex: both directions concurrently

Defaults per test/direction:

- total data: 1 MiB
- I/O chunk: 4096 bytes
- inactivity timeout: 60 seconds (reset whenever progress is made)
- pause between tests: 5 seconds

Relevant `prj.conf` values:

```ini
CONFIG_BENCH_TOTAL_BYTES=1048576
CONFIG_BENCH_CHUNK_SIZE=4096
CONFIG_BENCH_PAUSE_SECONDS=5
CONFIG_ECHO_SOCKET_TIMEOUT_MS=60000
```

For an initial cellular test, 1 MiB is sufficient. Increase to 4 or 8 MiB after the complete run succeeds so radio startup costs have less influence on throughput.
