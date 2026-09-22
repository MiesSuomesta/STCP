#!/usr/bin/env bash
set -euo pipefail
D=$(cd "$(dirname "$0")" && pwd)
RUN_DIR=${RUN_DIR:?set explicit RUN_DIR}
CASES=${CASES:-$D/cases.tsv}
HOST=${BENCHMARK_HOST:-192.168.1.20}

while IFS=$'\t' read -r case_id enabled mode port clients payload pipeline duration direction; do
  [[ $case_id == case_id || $case_id == \#* || $enabled != 1 ]] && continue
  "$D/run-case.sh" "$RUN_DIR" "$case_id" "$mode" "$HOST" "$port" "$clients" "$payload" "$pipeline" "$duration" "$direction"
done < "$CASES"
