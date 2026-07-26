# STCP Raspberry Pi benchmark website generator

Generates a static STCP.fi benchmark site for both STCP/TCP and STCP/UDP carrier matrices. No third-party Python or JavaScript packages are required.

## Generate TCP + UDP together

```bash
./generate-all.sh \
  /path/to/results/full-YYYYMMDD-HHMMSS/tcp \
  /path/to/results/full-YYYYMMDD-HHMMSS/udp \
  generated/raspberry-pi
```

Generated routes:

```text
generated/raspberry-pi/index.html      shared carrier landing page
generated/raspberry-pi/tcp/index.html  TCP carrier dashboard
generated/raspberry-pi/udp/index.html  UDP carrier dashboard
```

If no input directories are passed, `generate-all.sh` selects the newest `*-tcp` and `*-udp` result directories under `benchmark/results`.

```bash
RESULTS_ROOT=/path/to/benchmark/results ./generate-all.sh
```

## Generate one carrier only

```bash
./generate-and-publish-tcp.sh TCP_RESULT_DIR generated/raspberry-tcp
./generate-and-publish-udp.sh UDP_RESULT_DIR generated/raspberry-udp
```

The UDP page includes the STCP/UDP cases and the TCP/TLS reference baselines produced in the same matrix run. It excludes STCP/TCP cases. A future native `udp-c...json` baseline is also supported.

## Publish both dashboards

```bash
WEB_DEPLOY_TARGET='www-data@fuji:~/html/public/stcp.fi/benchmarks/raspberry-pi/' \
./publish-all.sh generated/raspberry-pi
```

Generate and publish in one command:

```bash
AUTO_PUBLISH_WEB=1 \
WEB_DEPLOY_TARGET='www-data@fuji:~/html/public/stcp.fi/benchmarks/raspberry-pi/' \
./generate-all.sh TCP_RESULT_DIR UDP_RESULT_DIR
```

Publishing validates SHA-256 hashes, uploads to a staging directory and atomically switches the complete Raspberry benchmark tree. The previous deployment remains in `.previous`.

## Full matrix integration

`benchmark/run-all-full.sh` now generates and publishes both pages automatically after TCP and UDP matrices complete:

```bash
AUTO_PUBLISH_WEB=1 \
WEB_DEPLOY_TARGET='www-data@fuji:~/html/public/stcp.fi/benchmarks/raspberry-pi/' \
RPI_HOST=192.168.1.199 \
RPI_SSH=pi@192.168.1.199 \
bash benchmark/run-all-full.sh
```

## Generated data

Each carrier dashboard contains:

- `dashboard-data.json`
- `summary.json`
- `cases.csv`
- `report.md`
- `manifest.json`
- `raw/` benchmark JSON, CSV, logs, perf and IRQ files

Headline comparisons use only successful cases with matching client, payload and pipeline dimensions. Failed STCP cases remain visible in the reliability section.
