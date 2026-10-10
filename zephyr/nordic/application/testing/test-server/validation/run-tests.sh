#!/bin/sh
set -eu
cd "$(dirname "$0")"
task_dir=$(mktemp -d)
trap 'rm -rf "$task_dir"' EXIT HUP INT TERM
cc -std=c11 -Wall -Wextra -Werror metrics_encode.c -o "$task_dir/encode"
"$task_dir/encode" > "$task_dir/metrics.bin"
python3 - "$task_dir/metrics.bin" <<'PY'
import struct,sys
b=open(sys.argv[1],'rb').read()
x=struct.unpack('>4I12Q',b)
assert x[:4]==(0x4d545232,1,112,2)
assert x[4]==0x0102030405060708 and x[-1]==0x1020304050607080
print('PASS: C metrics encoder / independent Python wire decoder')
PY
rustc --edition=2021 --test ../server/bench_protocol.rs -o "$task_dir/protocol"
BEN2_RESULTS="$task_dir/results.jsonl" "$task_dir/protocol" --test-threads=1
python3 - "$task_dir/results.jsonl" <<'PY'
import json,sys
rows=[json.loads(line) for line in open(sys.argv[1])]
assert rows and any(r['status']=='PASS' and r['client']['carrier']=='lte' for r in rows)
print('PASS: persisted JSONL parses; LTE carrier recorded')
PY
