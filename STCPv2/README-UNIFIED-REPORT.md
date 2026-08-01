# Unified STCPv2 benchmark report and publisher

A single English-language generator handles both Raspberry Pi and Zephyr result formats, normalizes them into one schema, and creates fully static HTML pages.

## Generate reports

```bash
bash generate-report.sh
```

Generated pages:

- `benchmark-report/site/latest/index.html` — combined comparison
- `RaspberryPI/benchmark/site/latest/index.html` — Raspberry Pi filtered view
- `zephyr/nordic/stcp-mqtt/benchmark/site/latest/index.html` — Zephyr filtered view

The report includes platform, carrier, transport, direction, payload, clients and pipeline filters. Raw JSON result sets are copied under `raw/`.

## Configure stcp.fi publication

```bash
cp .stcp-publish.env.example .stcp-publish.env
nano .stcp-publish.env
```

Example:

```bash
STCP_PUBLISH_HOST=stcp.fi
STCP_PUBLISH_USER=pomo
STCP_PUBLISH_PORT=22
STCP_PUBLISH_REMOTE_DIR=/var/www/stcp.fi/benchmarks
STCP_PUBLISH_KEEP_RELEASES=10
```

## Test publication locally

```bash
bash publish-stcp-fi.sh --local-target /tmp/stcp-benchmarks
firefox /tmp/stcp-benchmarks/latest/index.html
```

## Publish to stcp.fi

```bash
bash publish-stcp-fi.sh
```

By default, publication is refused when the newest combined report contains failed cases. Use `--allow-failures` only when publishing a diagnostic result set intentionally.

The publisher generates the report, validates it, creates a SHA-256 manifest, uploads a versioned release, atomically updates `latest/`, and retains previous releases under `releases/`.
