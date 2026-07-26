# STCP benchmark web publication orchestrator

This replaces the missing `publish-latest.sh`.

## Install

```bash
cp publish-benchmark-site.sh \
  ~/git/STCP/STCPv2/kernel-module/

chmod +x \
  ~/git/STCP/STCPv2/kernel-module/publish-benchmark-site.sh
```

Replace:

```bash
"$WEB_DIR/publish-latest.sh"
```

with:

```bash
"$ROOT/publish-benchmark-site.sh" "$RESULT_DIR"
```

## Publish latest results

```bash
./publish-benchmark-site.sh
```

## Publish named results

```bash
./publish-benchmark-site.sh \
  benchmark/results/full-20260726-122140
```

## Dry run

```bash
DRY_RUN=1 ./publish-benchmark-site.sh
```

## Commit, tag and push

```bash
AUTO_GIT=1 AUTO_PUSH=1 \
./publish-benchmark-site.sh \
  benchmark/results/full-20260726-122140
```

Default target:

```text
lja@fuji:/var/www/html/public/stcp.fi/benchmarks/raspberry-pi
```

Publication is atomic:

```text
raspberry-pi/
├── index.html
├── latest -> releases/<publish-id>
└── releases/<publish-id>/
```

Failed or incomplete result sets are not published.
