# STCP Raspberry/TCP benchmark dashboard generator

Generates a complete static STCP.fi benchmark page directly from benchmark result JSON files. It uses only Python's standard library and browser-native SVG.

## Generate

```bash
./generate-and-publish.sh \
  /home/pomo/git/STCP/STCPv2/kernel-module/benchmark/results/20260723-073008-tcp \
  generated/raspberry-tcp
```

Or call the generator directly:

```bash
python3 generate_dashboard.py RESULTS_DIR generated/raspberry-tcp \
  --platform "Raspberry Pi 5" \
  --transport tcp \
  --title "Raspberry Pi 5 / TCP benchmark" \
  --commit "$(git rev-parse --short HEAD)" \
  --kernel "$(ssh pi@192.168.1.199 uname -r)" \
  --compiler "$(aarch64-linux-gnu-gcc --version | head -1)"
```

Open locally:

```bash
xdg-open generated/raspberry-tcp/index.html
```

No local HTTP server is required because the generated data is embedded in `assets/data.js`.

## Publish to stcp.fi

```bash
WEB_DEPLOY_TARGET='www-data@fuji:~/html/public/stcp.fi/benchmarks/raspberry/tcp/' \
./publish.sh generated/raspberry-tcp
```

Generate and publish in one command:

```bash
AUTO_PUBLISH_WEB=1 \
WEB_DEPLOY_TARGET='www-data@fuji:~/html/public/stcp.fi/benchmarks/raspberry/tcp/' \
./generate-and-publish.sh RESULTS_DIR
```

The publisher validates SHA-256 hashes, uploads to a staging directory and switches directories atomically. The previous deployment is retained as `.previous`.

## Generated files

- `index.html`: interactive dashboard
- `dashboard-data.json`: complete normalized dataset
- `summary.json`: aggregate and pairwise statistics
- `cases.csv`: flat case table
- `report.md`: text summary
- `manifest.json`: SHA-256 manifest
- `raw/`: original JSON, CSV, perf and IRQ files

## Statistical policy

Headline comparisons use only directly matched successful cases with identical payload, client count and pipeline. Failed cases are not hidden: they are shown separately under Reliability / Known issues.
