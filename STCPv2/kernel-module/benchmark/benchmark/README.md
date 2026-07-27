# Summary + stcp.fi integration

Install:

```bash
cp benchmark/*.py benchmark/*.sh /your/project/benchmark/
chmod +x benchmark/*.py benchmark/*.sh
```

Generate a summary and site from old results without rerunning tests:

```bash
SITE_GENERATOR_CMD='your existing generator command using $RESULT_DIR and $OUTPUT_DIR' \
bash benchmark/generate-site.sh \
  benchmark/results/full-20260727-201455 \
  both
```

The generated site gets:

```text
pipeline-summary.json
index.html with an injected summary card
```

Displayed fields:

- run datestamp
- finish datestamp
- total wall duration
- configured duration per case
- average measured duration per case
- completed / expected cases
- PASS / FAIL / status
