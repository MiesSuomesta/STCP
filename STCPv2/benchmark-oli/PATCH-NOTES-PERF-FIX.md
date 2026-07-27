# perf collection fix

Fixed issues in `run-all.sh`:

- remote shell arguments are now safely quoted;
- startup failure is detected instead of treating every `nohup` launch as success;
- the remote perf error log is printed immediately when startup fails;
- each perf process is waited for after the benchmark case;
- perf CSV and diagnostic log are copied into the result directory;
- `enrich_perf_metrics.py` is invoked for every successful measurement;
- temporary Raspberry Pi perf files are removed after collection;
- `PERF_STARTUP_WAIT_SECONDS` can tune the startup validation delay.
