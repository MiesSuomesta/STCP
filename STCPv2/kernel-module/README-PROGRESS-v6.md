# v6 progress reporting

## Zephyr / modem

Default interval in `prj.conf`:

```ini
CONFIG_BENCH_REPORT_INTERVAL_MS=5000
```

Runtime shell setting:

```text
stcp config report 5000
stcp config show
```

Set `0` to disable periodic reports. A final 100% line is always printed.

Example:

```text
UPLOAD TX progress=42% transferred=442368/1048576 bytes remaining=606208 rate=183420 bit/s
DOWNLOAD RX progress=77% transferred=811008/1048576 bytes remaining=237568 rate=241901 bit/s
FULL TX progress=... 
FULL RX progress=...
```

## Echo server

```bash
sudo python3 echo_server.py --no-tls --no-stcp --verbose --report-every 5
```

Set `--report-every 0` to disable periodic reports. Final 100% progress is still logged.
