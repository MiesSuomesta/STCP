#!/usr/bin/env bash
set -Eeuo pipefail
export PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:${PATH:-}"
D="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
STAMP="$(date +%Y%m%d-%H%M%S)"
ROOT_OUT="${FULL_RESULT_DIR:-$D/results/full-$STAMP}"
FULL_LOG="$ROOT_OUT/full-run.log"
RPI_HOST="${RPI_HOST:-192.168.1.50}"
RPI_SSH="${RPI_SSH:-pi@$RPI_HOST}"
RPI_BENCHMARK_DIR="${RPI_BENCHMARK_DIR:-/home/pi/benchmark}"
SSH_OPTS="${SSH_OPTS:--o StrictHostKeyChecking=accept-new}"
CONTINUE_ON_ERROR="${CONTINUE_ON_ERROR:-0}"
SYNC_RPI="${SYNC_RPI:-1}"
mkdir -p "$ROOT_OUT"
exec > >(tee -a "$FULL_LOG") 2>&1
printf 'STCP full benchmark\nStarted: %s\nRaspberry: %s\nResults: %s\n\n' "$(date --iso-8601=seconds)" "$RPI_SSH" "$ROOT_OUT"
sync_raspberry(){
  [[ "$SYNC_RPI" == 1 ]] || return 0
  echo "[INFO] Synchronizing benchmark scripts to Raspberry"
  ssh $SSH_OPTS "$RPI_SSH" "mkdir -p '$RPI_BENCHMARK_DIR'"
  tar -C "$D" -czf - benchmark_server.py start-servers.sh stop-servers.sh irq_snapshot.py README.md | ssh $SSH_OPTS "$RPI_SSH" "tar -C '$RPI_BENCHMARK_DIR' -xzf -"
}
sync_raspberry
remote_servers(){
  local transport="$1"
  ssh $SSH_OPTS "$RPI_SSH" "cd '$RPI_BENCHMARK_DIR' && (bash stop-servers.sh || true) && STCP_TRANSPORT='$transport' bash start-servers.sh"
}
run_transport(){
  local transport="$1" out="$ROOT_OUT/$1" log="$ROOT_OUT/$1.log" rc
  mkdir -p "$out"
  echo "============================================================"
  echo " STCP carrier: ${transport^^}"
  echo "============================================================"
  remote_servers "$transport"; sleep 2
  set +e
  STCP_TRANSPORT="$transport" RESULT_DIR="$out" RPI_HOST="$RPI_HOST" DURATION="${DURATION:-30}" CLIENTS_LIST="${CLIENTS_LIST:-1 2 4 8}" PAYLOADS="${PAYLOADS:-64 1024 4096 65536 262144 1048576}" PIPELINES="${PIPELINES:-1 4 8}" VERIFY="${VERIFY:-0}" IRQ_METRICS="${IRQ_METRICS:-1}" PERF_METRICS="${PERF_METRICS:-1}" PERF_EVENTS="${PERF_EVENTS:-task-clock,context-switches,cpu-migrations,page-faults,cycles,instructions,branches,branch-misses,cache-references,cache-misses}" REMOTE_PERF_PREFIX="${REMOTE_PERF_PREFIX:-sudo -n}" RPI_SSH="$RPI_SSH" RPI_BENCHMARK_DIR="$RPI_BENCHMARK_DIR" SSH_OPTS="$SSH_OPTS" bash "$D/run-all.sh" |& tee "$log"
  rc=${PIPESTATUS[0]}; set -e
  if ((rc!=0)); then
    echo "[FAIL] $transport matrix exited with status $rc"
    [[ "$CONTINUE_ON_ERROR" == 1 ]] || return "$rc"
  else echo "[ OK ] $transport matrix completed"; fi
}
run_transport tcp
run_transport udp
ssh $SSH_OPTS "$RPI_SSH" "cd '$RPI_BENCHMARK_DIR' && bash stop-servers.sh" || true
python3 - "$ROOT_OUT" <<'PYCOMBINE'
import csv,json,sys
from pathlib import Path
root=Path(sys.argv[1]); rows=[]
for t in ('tcp','udp'):
 p=root/t/'summary.json'
 if p.exists(): rows.extend(json.loads(p.read_text()).get('results',[]))
(root/'summary.json').write_text(json.dumps({'tests':len(rows),'results':rows},indent=2)+'\n')
fields=['mode','transport','clients','payload_bytes','pipeline','elapsed_s','operations','errors','combined_mib_s','operations_s','connect_mean_ms','rtt_p50_ms','rtt_p95_ms','rtt_p99_ms','client_cpu_percent','max_rss_kib','server_cpu_busy_percent','server_network_irq','server_net_rx_softirq','server_net_tx_softirq','server_network_irq_per_1k_ops','server_net_rx_softirq_per_1k_ops','server_net_tx_softirq_per_1k_ops','server_kernel_network_events_per_1k_ops','server_network_irq_per_mib','server_perf_task_clock_ms','server_perf_context_switches','server_perf_cpu_migrations','server_perf_page_faults','server_perf_cycles','server_perf_instructions','server_perf_branches','server_perf_branch_misses','server_perf_cache_references','server_perf_cache_misses','server_perf_cycles_per_op','server_perf_instructions_per_op','server_perf_context_switches_per_1k_ops','server_perf_cpu_migrations_per_1k_ops','server_perf_page_faults_per_1k_ops','server_perf_cycles_per_mib','server_perf_instructions_per_mib','server_perf_task_clock_ms_per_1k_ops','server_perf_ipc','server_perf_branch_miss_percent','server_perf_cache_miss_percent']
with (root/'results.csv').open('w',newline='') as f:
 w=csv.DictWriter(f,fieldnames=fields,extrasaction='ignore'); w.writeheader(); w.writerows(rows)
def label(r): return f"STCP/{r.get('transport','tcp').upper()}" if r.get('mode')=='stcp' else r.get('mode','').upper()
L=['# Full Raspberry Pi benchmark: STCP/TCP and STCP/UDP','',f'Tests: **{len(rows)}**','', '| Mode | Clients | Payload | Pipe | Combined MiB/s | Ops/s | RTT p50 ms | Connect ms | CPU % | Errors |','|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|']
for r in sorted(rows,key=lambda x:(x.get('payload_bytes',0),x.get('clients',0),x.get('pipeline',0),label(x))):
 L.append(f"| {label(r)} | {r['clients']} | {r['payload_bytes']} | {r['pipeline']} | {r['combined_mib_s']:.2f} | {r['operations_s']:.2f} | {r['rtt_p50_ms']:.3f} | {r['connect_mean_ms']:.3f} | {r['client_cpu_percent']:.1f} | {r['errors']} |")
(root/'report.md').write_text('\n'.join(L)+'\n')
PYCOMBINE
echo "Full benchmark complete: $ROOT_OUT"
echo "Report: $ROOT_OUT/report.md"
echo "CSV:    $ROOT_OUT/results.csv"
echo "Log:    $FULL_LOG"


if [[ "${AUTO_PUBLISH_WEB:-0}" == 1 ]]; then
  echo "[INFO] Generating and publishing Raspberry TCP + UDP dashboards to stcp.fi"
  WEB_DEPLOY_TARGET="${WEB_DEPLOY_TARGET:-www-data@fuji:~/html/public/stcp.fi/benchmarks/raspberry-pi/}" \
  PLATFORM_NAME="${PLATFORM_NAME:-Raspberry Pi}" \
  BENCHMARK_KERNEL="${BENCHMARK_KERNEL:-unknown}" \
  BENCHMARK_COMPILER="${BENCHMARK_COMPILER:-unknown}" \
  AUTO_PUBLISH_WEB=1 \
  bash "$D/stcp-raspberry-tcp-generator/generate-all.sh" \
    "$ROOT_OUT/tcp" "$ROOT_OUT/udp" \
    "${WEB_OUTPUT_DIR:-$ROOT_OUT/web/raspberry-pi}"
fi
