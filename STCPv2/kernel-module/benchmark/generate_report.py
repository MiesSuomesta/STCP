#!/usr/bin/env python3
import argparse,json,csv
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument('--input-dir',required=True); a=p.parse_args(); d=Path(a.input_dir); rows=[]
for f in sorted(d.glob('*.json')):
    try:r=json.loads(f.read_text())
    except:continue
    if 'mode' in r:rows.append(r)
rows.sort(key=lambda r:(r['payload_bytes'],r['clients'],r['pipeline'],r['mode']))
fields=['mode','clients','payload_bytes','pipeline','elapsed_s','operations','errors','combined_mib_s','operations_s','connect_mean_ms','rtt_p50_ms','rtt_p95_ms','rtt_p99_ms','client_cpu_percent','max_rss_kib']
with open(d/'results.csv','w',newline='') as h:
    w=csv.DictWriter(h,fieldnames=fields,extrasaction='ignore'); w.writeheader(); w.writerows(rows)
L=['# Raspberry Pi TCP vs TLS vs STCP benchmark','','| Mode | Clients | Payload | Pipe | MiB/s combined | Ops/s | RTT p50 | RTT p95 | RTT p99 | CPU % | Errors |','|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|']
for r in rows:L.append(f"| {r['mode'].upper()} | {r['clients']} | {r['payload_bytes']} | {r['pipeline']} | {r['combined_mib_s']:.2f} | {r['operations_s']:.2f} | {r['rtt_p50_ms']:.2f} | {r['rtt_p95_ms']:.2f} | {r['rtt_p99_ms']:.2f} | {r['client_cpu_percent']:.1f} | {r['errors']} |")
(d/'report.md').write_text('\n'.join(L)+'\n'); (d/'summary.json').write_text(json.dumps({'tests':len(rows),'results':rows},indent=2)+'\n')
