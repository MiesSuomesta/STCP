#!/usr/bin/env bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
CASES=${CASES:-$D/cases.tsv}
RUN_DIR=${RUN_DIR:?set explicit RUN_DIR; latest is deliberately not inferred}
DATABASE=${DATABASE:-$D/benchmark-history.sqlite3}
exec 9>"${BENCHMARK_LOCK:-/tmp/stcp-benchmark-v2.lock}"
flock -n 9 || { echo "another benchmark-v2 run is active" >&2; exit 1; }

python3 "$D/benchctl.py" init --run-dir "$RUN_DIR" --cases "$CASES"
"$D/collect-environment.sh" "$RUN_DIR"
"$D/run-matrix.sh"
python3 "$D/benchctl.py" validate --run-dir "$RUN_DIR"
python3 "$D/benchctl.py" ingest --run-dir "$RUN_DIR" --database "$DATABASE"
python3 "$D/benchctl.py" report --run-dir "$RUN_DIR"
python3 "$D/benchctl.py" approve --run-dir "$RUN_DIR"
echo "PASS and publication-approved: $RUN_DIR"
