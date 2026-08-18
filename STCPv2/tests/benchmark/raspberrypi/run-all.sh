#!/usr/bin/env bash
set -euo pipefail
D="$(cd "$(dirname "$0")" && pwd)"; H=${RPI_HOST:-192.168.1.50}; O=${RESULT_DIR:-$D/results/$(date +%Y%m%d-%H%M%S)}; mkdir -p "$O"
DU=${DURATION:-30}; CL=${CLIENTS_LIST:-'1 2 4 8'}; PL=${PAYLOADS:-'64 1024 4096 65536 262144 1048576'}; Q=${PIPELINES:-'1 4 8'}; V=(); [ "${VERIFY:-0}" = 1 ] && V=(--verify)
run(){ m=$1;p=$2;c=$3;s=$4;q=$5; n="$m-c$c-p$s-q$q"; echo "===== $n ====="; python3 "$D/benchmark_client.py" --mode "$m" --host "$H" --port "$p" --clients "$c" --payload "$s" --pipeline "$q" --duration "$DU" --output-json "$O/$n.json" "${V[@]}"; }
for s in $PL; do for c in $CL; do for q in $Q; do run tcp 19000 $c $s $q; run tls 19001 $c $s $q; run stcp 19002 $c $s $q; done; done; done
python3 "$D/generate_report.py" --input-dir "$O"; echo "Reports: $O/report.md $O/results.csv $O/summary.json"
