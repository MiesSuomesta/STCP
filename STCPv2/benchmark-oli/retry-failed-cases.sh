#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SOURCE_RESULT="${1:-}"
SELECTED_CARRIERS="${2:-udp tcp}"
RESULTS_ROOT="${RESULTS_ROOT:-$SCRIPT_DIR/results}"
PIPELINE="${PIPELINE:-$ROOT/build-benchmark-publish.sh}"
MAX_RETRIES="${MAX_RETRIES:-5}"
RETRY_DELAY="${RETRY_DELAY:-3}"
DURATION="${DURATION:-15}"
PERF_METRICS="${PERF_METRICS:-1}"
IRQ_METRICS="${IRQ_METRICS:-1}"
VERIFY="${VERIFY:-0}"
SYNC_RPI="${SYNC_RPI:-1}"
CONTINUE_ON_ERROR="${CONTINUE_ON_ERROR:-1}"
RPI_ADDR="${RPI_ADDR:-192.168.1.199}"
RPI_USER="${RPI_USER:-pi}"
RPI_BENCHMARK_DIR="${RPI_BENCHMARK_DIR:-/home/pi/benchmark}"
STAMP="${STAMP:-$(date +%Y%m%d-%H%M%S)}"

log(){ printf '[INFO] %s\n' "$*"; }
ok(){ printf '[ OK ] %s\n' "$*"; }
warn(){ printf '[WARN] %s\n' "$*" >&2; }
die(){ printf '[FAIL] %s\n' "$*" >&2; exit 1; }

latest_after(){
  find "$RESULTS_ROOT" -mindepth 1 -maxdepth 1 -type d -name 'full-*' \
    -newermt "@$1" -printf '%T@ %p\n' 2>/dev/null | sort -nr | head -1 | cut -d' ' -f2-
}

case_ok(){
  python3 - "$1" <<'PY'
import json,sys
from pathlib import Path
p=Path(sys.argv[1])
try:
 d=json.loads(p.read_text())
 ok=int(d.get('errors',0))==0 and int(d.get('operations',0))>0 and not (d.get('error_details') or [])
except Exception:
 ok=False
raise SystemExit(0 if ok else 1)
PY
}

annotate(){
  python3 - "$1" "$2" "$MAX_RETRIES" "$3" <<'PY'
import json,sys
from pathlib import Path
p=Path(sys.argv[1]); d=json.loads(p.read_text())
d['benchmark_retried']=True
d['benchmark_retry_attempt']=int(sys.argv[2])
d['benchmark_max_retries']=int(sys.argv[3])
d['benchmark_original_failure']=sys.argv[4]
t=p.with_suffix(p.suffix+'.tmp'); t.write_text(json.dumps(d,indent=2)+'\n'); t.replace(p)
PY
}

[[ -n "$SOURCE_RESULT" ]] || die "Usage: $0 /path/to/full-result [udp|tcp|both]"
SOURCE_RESULT="$(cd "$SOURCE_RESULT" && pwd)"
case "$SELECTED_CARRIERS" in
  udp) SELECTED_CARRIERS=udp ;;
  tcp) SELECTED_CARRIERS=tcp ;;
  both|'udp tcp'|'tcp udp') SELECTED_CARRIERS='udp tcp' ;;
  *) die "Invalid carriers: $SELECTED_CARRIERS" ;;
esac
[[ "$MAX_RETRIES" =~ ^[0-9]+$ ]] || die "MAX_RETRIES must be integer"
(( MAX_RETRIES > 0 )) || { warn "Retries disabled"; exit 0; }
[[ -f "$PIPELINE" ]] || die "Missing pipeline: $PIPELINE"

RETRY_DIR="$SOURCE_RESULT/retries/$STAMP"
FAILED="$RETRY_DIR/failed.tsv"
HISTORY="$SOURCE_RESULT/RETRY-HISTORY.tsv"
UNRESOLVED="$SOURCE_RESULT/UNRESOLVED-RETRY-CASES.tsv"
mkdir -p "$RETRY_DIR"
: > "$FAILED"; : > "$UNRESOLVED"; touch "$HISTORY"

python3 - "$SOURCE_RESULT" "$SELECTED_CARRIERS" "$FAILED" <<'PY'
import json,re,sys
from pathlib import Path
root=Path(sys.argv[1]); carriers=sys.argv[2].split(); out=Path(sys.argv[3])
rx={
 'udp':re.compile(r'^(udp|tls|stcp-udp)-c(\d+)-p(\d+)-q(\d+)\.json$'),
 'tcp':re.compile(r'^(tcp|tls|stcp-tcp)-c(\d+)-p(\d+)-q(\d+)\.json$')}
rows=[]
for c in carriers:
 d=root/c
 if not d.is_dir(): continue
 for p in sorted(d.iterdir()):
  m=rx[c].fullmatch(p.name)
  if not m: continue
  mode,cl,pa,pi=m.groups(); reasons=[]
  try: data=json.loads(p.read_text())
  except Exception as e: data={}; reasons.append('invalid-json:'+type(e).__name__)
  if data:
   try:
    if int(data.get('errors',0))!=0: reasons.append('errors='+str(data.get('errors')))
   except Exception: reasons.append('invalid-errors')
   try:
    if int(data.get('operations',0))<=0: reasons.append('operations='+str(data.get('operations')))
   except Exception: reasons.append('invalid-operations')
   if data.get('error_details') or []: reasons.append('details='+repr(data.get('error_details'))[:250])
  if reasons: rows.append((c,mode,cl,pa,pi,';'.join(reasons),str(p)))
out.write_text(''.join('\t'.join(r)+'\n' for r in rows))
print(len(rows))
PY

COUNT=$(wc -l < "$FAILED" | tr -d '[:space:]')
if [[ "$COUNT" == 0 ]]; then ok "No failed cases"; rm -f "$UNRESOLVED"; exit 0; fi
log "Found $COUNT failed case(s)"
TUPLES="$RETRY_DIR/tuples.tsv"
cut -f1,3,4,5 "$FAILED" | sort -u > "$TUPLES"

unresolved=0
while IFS=$'\t' read -r carrier clients payload pipeline; do
  tuple_failed="$RETRY_DIR/${carrier}-c${clients}-p${payload}-q${pipeline}.tsv"
  awk -F'\t' -v c="$carrier" -v n="$clients" -v p="$payload" -v q="$pipeline" \
    '$1==c && $3==n && $4==p && $5==q' "$FAILED" > "$tuple_failed"
  fixed=0
  for ((attempt=1; attempt<=MAX_RETRIES; attempt++)); do
    log "Retry $attempt/$MAX_RETRIES: $carrier c=$clients p=$payload q=$pipeline"
    started=$(date +%s)
    set +e
    CARRIERS="$carrier" STCP_CARRIERS="$carrier" DURATION="$DURATION" \
    CLIENTS_LIST="$clients" PAYLOADS="$payload" PIPELINES="$pipeline" \
    PERF_METRICS="$PERF_METRICS" IRQ_METRICS="$IRQ_METRICS" VERIFY="$VERIFY" \
    SYNC_RPI="$SYNC_RPI" CONTINUE_ON_ERROR="$CONTINUE_ON_ERROR" AUTO_PUBLISH_WEB=0 \
    RPI_ADDR="$RPI_ADDR" RPI_USER="$RPI_USER" RPI_BENCHMARK_DIR="$RPI_BENCHMARK_DIR" \
      bash "$PIPELINE" benchmark
    set -e
    retry_result=$(latest_after "$started")
    all_ok=1
    while IFS=$'\t' read -r _ mode _ _ _ reason original; do
      base=$(basename "$original")
      retry_json="$retry_result/$carrier/$base"
      attempt_dir="$RETRY_DIR/$carrier/c${clients}-p${payload}-q${pipeline}/attempt-${attempt}/$mode"
      mkdir -p "$attempt_dir"
      if [[ -n "$retry_result" ]] && case_ok "$retry_json"; then
        stem=${base%.json}
        find "$retry_result/$carrier" -maxdepth 1 -type f \
          \( -name "$stem.json" -o -name "$stem.*" \) -exec cp -a {} "$attempt_dir/" \;
        find "$retry_result/$carrier" -maxdepth 1 -type f \
          \( -name "$stem.json" -o -name "$stem.*" \) -exec cp -a {} "$SOURCE_RESULT/$carrier/" \;
        annotate "$SOURCE_RESULT/$carrier/$base" "$attempt" "$reason"
        printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\tpass\t%s\n' "$(date --iso-8601=seconds)" "$carrier" "$clients" "$payload" "$pipeline" "$mode" "$attempt" "$retry_result" >> "$HISTORY"
      else
        all_ok=0
        [[ -f "$retry_json" ]] && cp -a "$retry_json" "$attempt_dir/" || true
        printf '%s\t%s\t%s\t%s\t%s\t%s\t%s\tfail\t%s\n' "$(date --iso-8601=seconds)" "$carrier" "$clients" "$payload" "$pipeline" "$mode" "$attempt" "${retry_result:-none}" >> "$HISTORY"
      fi
    done < "$tuple_failed"
    if (( all_ok )); then ok "Tuple recovered on attempt $attempt"; fixed=1; break; fi
    (( attempt < MAX_RETRIES )) && sleep "$RETRY_DELAY"
  done
  if (( ! fixed )); then cat "$tuple_failed" >> "$UNRESOLVED"; unresolved=$((unresolved+1)); fi
done < "$TUPLES"

if (( unresolved > 0 )); then warn "$unresolved tuple(s) still failing"; exit 1; fi
rm -f "$UNRESOLVED"
ok "All failed tuples recovered"
