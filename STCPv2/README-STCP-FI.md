# Unified stcp.fi benchmark pages

This package leaves the existing stcp.fi home page untouched and replaces only `/benchmarks/`.

Generate all pages:

```bash
bash generate-report.sh
```

Generated pages:

- `stcp.fi/benchmarks/index.html`
- `stcp.fi/benchmarks/latest/index.html`
- `stcp.fi/benchmarks/latest/raspberry-pi.html`
- `stcp.fi/benchmarks/latest/zephyr.html`

Publish:

```bash
cp .stcp-publish.env.example .stcp-publish.env
nano .stcp-publish.env
bash publish-stcp-fi.sh
```
