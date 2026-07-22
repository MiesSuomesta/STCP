#!/usr/bin/env python3
import argparse,json,csv
from pathlib import Path
p=argparse.ArgumentParser(); p.add_argument('--input-dir',required=True); a=p.parse_args(); d=Path(a.input_dir); rows=[]
for f in sorted(d.glob('*.json')):
    if f.name=='summary.json': continue
    try:r=json.loads(f.read_text())
    except:continue
    if 'mode' in r:rows.append(r)
def label(r): return f"STCP/{r.get('transport','tcp').upper()}" if r['mode']=='stcp' else r['mode'].upper()
rows.sort(key=lambda r:(r['payload_bytes'],r['clients'],r['pipeline'],label(r)))
fields=['mode','transport','clients','payload_bytes','pipeline','elapsed_s','operations','errors','combined_mib_s','operations_s','connect_mean_ms','rtt_p50_ms','rtt_p95_ms','rtt_p99_ms','client_cpu_percent','max_rss_kib','server_cpu_busy_percent','server_network_irq','server_net_rx_softirq','server_net_tx_softirq','server_network_irq_per_1k_ops','server_net_rx_softirq_per_1k_ops','server_net_tx_softirq_per_1k_ops','server_kernel_network_events_per_1k_ops','server_network_irq_per_mib','server_perf_task_clock_ms','server_perf_context_switches','server_perf_cpu_migrations','server_perf_page_faults','server_perf_cycles','server_perf_instructions','server_perf_branches','server_perf_branch_misses','server_perf_cache_references','server_perf_cache_misses','server_perf_cycles_per_op','server_perf_instructions_per_op','server_perf_context_switches_per_1k_ops','server_perf_cpu_migrations_per_1k_ops','server_perf_page_faults_per_1k_ops','server_perf_cycles_per_mib','server_perf_instructions_per_mib','server_perf_task_clock_ms_per_1k_ops','server_perf_ipc','server_perf_branch_miss_percent','server_perf_cache_miss_percent']
with open(d/'results.csv','w',newline='') as h:
    w=csv.DictWriter(h,fieldnames=fields,extrasaction='ignore'); w.writeheader(); w.writerows(rows)
L=['# Raspberry Pi TCP vs TLS vs STCP benchmark','','| Mode | Clients | Payload | Pipe | MiB/s combined | Ops/s | RTT p50 | RTT p95 | RTT p99 | Connect ms | CPU % | Server busy % | Net IRQ/1k ops | NET_RX/1k ops | NET_TX/1k ops | Kernel events/1k ops | Cycles/op | Instr/op | Ctx sw/1k ops | IPC | Cache miss % | Errors |','|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|']
for r in rows:L.append(f"| {label(r)} | {r['clients']} | {r['payload_bytes']} | {r['pipeline']} | {r['combined_mib_s']:.2f} | {r['operations_s']:.2f} | {r['rtt_p50_ms']:.3f} | {r['rtt_p95_ms']:.3f} | {r['rtt_p99_ms']:.3f} | {r['connect_mean_ms']:.3f} | {r['client_cpu_percent']:.1f} | {r.get('server_cpu_busy_percent',0):.1f} | {r.get('server_network_irq_per_1k_ops',0):.3f} | {r.get('server_net_rx_softirq_per_1k_ops',0):.3f} | {r.get('server_net_tx_softirq_per_1k_ops',0):.3f} | {r.get('server_kernel_network_events_per_1k_ops',0):.3f} | {r.get('server_perf_cycles_per_op') or 0:.1f} | {r.get('server_perf_instructions_per_op') or 0:.1f} | {r.get('server_perf_context_switches_per_1k_ops') or 0:.3f} | {r.get('server_perf_ipc') or 0:.3f} | {r.get('server_perf_cache_miss_percent') or 0:.3f} | {r['errors']} |")
(d/'report.md').write_text('\n'.join(L)+'\n'); (d/'summary.json').write_text(json.dumps({'tests':len(rows),'results':rows},indent=2)+'\n')
