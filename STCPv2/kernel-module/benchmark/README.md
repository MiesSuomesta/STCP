# Benchmark pipeline v2

The pipeline has four independent responsibilities:

1. `benchmark/run-case.sh`
   Runs exactly one case. The first run is normal. On failure it retries five
   times by default, restarting the benchmark servers before every retry.

2. `benchmark/run-all.sh`
   Builds or reads the complete case matrix and calls `run-case.sh` for every
   case. It creates one explicit `full-YYYYmmdd-HHMMSS` result directory.

3. `benchmark/generate-site.sh`
   Receives an explicit result directory and generates a new `/tmp/www-$$`
   directory. It never touches the live site.

4. `benchmark/deploy-site.sh`
   Receives the generated directory, backs up the live site and replaces it
   atomically.

## Typical flow

```bash
bash benchmark/run-all.sh both

bash benchmark/generate-site.sh \
  benchmark/results/full-20260727-190000 both

sudo bash benchmark/deploy-site.sh \
  /tmp/www-12345 /var/www/stcp.fi
```

## Exact case matrix

For the production benchmark matrix, use a TSV file:

```text
kind	clients	payload	pipeline	duration
tcp	1	64	1	15
tls	1	64	1	15
stcp-tcp	1	64	1	15
udp	1	64	1	15
stcp-udp	1	64	1	15
```

Run it with:

```bash
CASE_FILE=benchmark/cases.tsv \
  bash benchmark/run-all.sh both
```

This keeps the production matrix explicit and version-controlled.


## Complete orchestration

`benchmark/orchestrate-benchmarks.sh` runs the complete pipeline:

```bash
CASE_FILE=benchmark/cases.tsv \
SITE_GENERATOR=/path/to/generator.py \
sudo -E bash benchmark/orchestrate-benchmarks.sh \
  both /var/www/stcp.fi
```

It uses an exclusive lock, preserves results on failure and removes the
temporary generated site after a successful deployment.

Resume from an existing result set:

```bash
SKIP_BENCHMARKS=1 \
RESULT_DIR=benchmark/results/full-20260727-190000 \
SITE_GENERATOR=/path/to/generator.py \
sudo -E bash benchmark/orchestrate-benchmarks.sh \
  both /var/www/stcp.fi
```

Generate without publishing:

```bash
SKIP_DEPLOY=1 KEEP_GENERATED=1 \
bash benchmark/orchestrate-benchmarks.sh \
  both /var/www/stcp.fi
```
