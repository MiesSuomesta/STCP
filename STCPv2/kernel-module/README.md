# STCP Raspberry restart before every real benchmark case

Install from the `kernel-module` root:

```bash
cp run-full-benchmark.sh .
cp benchmark/orchestrate-stcp-udp-tests.sh benchmark/
cp benchmark/restart-rpi-benchmark-servers.sh benchmark/

chmod +x \
  run-full-benchmark.sh \
  benchmark/orchestrate-stcp-udp-tests.sh \
  benchmark/restart-rpi-benchmark-servers.sh
```

Run:

```bash
RESTART_SERVERS_EACH_CASE=1 \
SERVER_RESTART_DELAY=2 \
bash run-full-benchmark.sh
```

The case loop is inside `TEST_SCRIPT`, so restarting only in `run_case()` is
not sufficient. The corrected orchestrator prepends a `python3` wrapper to
`PATH`. Every invocation whose arguments contain `benchmark_client.py` calls
the Raspberry restart helper first.

The helper verifies that the Raspberry server PID set changed. A failed
restart aborts that benchmark invocation instead of silently continuing.

Expected output before every client invocation:

```text
[CASE-RESTART] Restarting Raspberry servers...
[CASE-RESTART] PID set changed successfully
[CASE-RESTART] Before: ...
[CASE-RESTART] After:  ...
```

Disable:

```bash
RESTART_SERVERS_EACH_CASE=0 bash run-full-benchmark.sh
```
