#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
MODE="${1:-both}"
RESULTS_ROOT="${RESULTS_ROOT:-$SCRIPT_DIR/results}"
PUBLISH_SCRIPT="${PUBLISH_SCRIPT:-$SCRIPT_DIR/publish-benchmark-site.sh}"
MIN_DURATION_RATIO="${MIN_DURATION_RATIO:-0.90}"

log(){ printf '[INFO] %s\n' "$*"; }
ok(){ printf '[ OK ] %s\n' "$*"; }
die(){ printf '[FAIL] %s\n' "$*" >&2; exit 1; }
case "$MODE" in tcp|udp|both) ;; *) die "Usage: $0 [tcp|udp|both]";; esac
command -v python3 >/dev/null || die "python3 is required"
[[ -x "$PUBLISH_SCRIPT" ]] || die "Publish script missing: $PUBLISH_SCRIPT"

latest="$(find "$RESULTS_ROOT" -mindepth 1 -maxdepth 1 -type d -name 'full-*' -printf '%T@ %p\n' 2>/dev/null | sort -nr | awk 'NR==1{sub(/^[^ ]+ /,"");print}')"
[[ -n "$latest" ]] || die "No full-* result set found below $RESULTS_ROOT"
RESULT_DIR="$(cd -- "$latest" && pwd -P)"
log "Selected mode: $MODE"
log "Result set:    $RESULT_DIR"

python3 - "$RESULT_DIR" "$MODE" "$MIN_DURATION_RATIO" <<'PY'
import csv,json,re,sys
from pathlib import Path
root=Path(sys.argv[1]); mode=sys.argv[2]; ratio=float(sys.argv[3])
kinds={'tcp':{'tcp','tls','stcp-tcp'},'udp':{'udp','stcp-udp'}}
selected=['tcp','udp'] if mode=='both' else [mode]
case_re=re.compile(r'^(tcp|tls|stcp-tcp|udp|stcp-udp)-c(\d+)-p(\d+)-q(\d+)\.json$')
manifest={m:[] for m in selected}
cases_file=root/'cases.tsv'
if cases_file.is_file():
    with cases_file.open(encoding='utf-8',newline='') as f:
        for row in csv.DictReader(f,delimiter='\t'):
            kind=(row.get('kind') or '').strip()
            for m in selected:
                if kind in kinds[m]: manifest[m].append(row)
for m in selected:
    files=[]
    for p in sorted((root/m).glob('*.json')):
        mt=case_re.match(p.name)
        if mt and mt.group(1) in kinds[m]: files.append(p)
    expected=len(manifest[m]) or len(files)
    errors=[]
    if len(files)!=expected: errors.append(f'expected {expected} primary JSON files, found {len(files)}')
    durations={}
    for row in manifest[m]:
        try: durations[(row['kind'],int(row['clients']),int(row['payload']),int(row['pipeline']))]=float(row.get('duration') or 0)
        except Exception: pass
    for p in files:
        try: d=json.loads(p.read_text())
        except Exception as e: errors.append(f'{p.name}: invalid JSON: {e}'); continue
        mt=case_re.match(p.name); key=(mt.group(1),int(mt.group(2)),int(mt.group(3)),int(mt.group(4)))
        if int(d.get('errors',0) or 0)!=0: errors.append(f'{p.name}: errors={d.get("errors")}')
        if int(d.get('operations',0) or 0)<=0: errors.append(f'{p.name}: operations={d.get("operations")}')
        if d.get('error_details') not in (None,[]): errors.append(f'{p.name}: error_details not empty')
        required=durations.get(key,0)*ratio
        if required and float(d.get('elapsed_s',0) or 0)<required: errors.append(f'{p.name}: elapsed_s below {required:.1f}')
    if errors:
        print(f'[FAIL] {m} validation failed:',file=sys.stderr)
        for e in errors[:30]: print(f'  - {e}',file=sys.stderr)
        if len(errors)>30: print(f'  ... and {len(errors)-30} more',file=sys.stderr)
        raise SystemExit(1)
    print(f'[ OK ] Validated {len(files)} {m} benchmark case files')
PY

bash "$PUBLISH_SCRIPT" "$RESULT_DIR" "$MODE"
ok "Published latest benchmark result set"
