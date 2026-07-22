#!/usr/bin/env bash
set -Eeuo pipefail
D="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
H="${RPI_HOST:-192.168.1.50}"
T="${STCP_TRANSPORT:-tcp}"
DU="${DURATION:-30}"
CL="${CLIENTS_LIST:-1 2 4 8}"
PL="${PAYLOADS:-64 1024 4096 65536 262144 1048576}"
Q="${PIPELINES:-1 4 8}"
TCP_PORT="${TCP_PORT:-19000}"
TLS_PORT="${TLS_PORT:-19001}"
STCP_PORT="${STCP_PORT:-19002}"
O="${RESULT_DIR:-$D/results/$(date +%Y%m%d-%H%M%S)-$T}"
RPI_SSH="${RPI_SSH:-pi@$H}"
RPI_BENCHMARK_DIR="${RPI_BENCHMARK_DIR:-/home/pi/benchmark}"
SSH_OPTS="${SSH_OPTS:--o StrictHostKeyChecking=accept-new}"
IRQ_METRICS="${IRQ_METRICS:-1}"
IRQ_NETWORK_PATTERN="${IRQ_NETWORK_PATTERN:-(eth|enp|eno|end|bcmgenet|genet|lan|wlan|wifi|brcm|dwc|fec|gmac|eqos)}"
PERF_METRICS="${PERF_METRICS:-1}"
PERF_EVENTS="${PERF_EVENTS:-task-clock,context-switches,cpu-migrations,page-faults,cycles,instructions,branches,branch-misses,cache-references,cache-misses}"
PERF_GRACE_SECONDS="${PERF_GRACE_SECONDS:-2}"
REMOTE_PERF_PREFIX="${REMOTE_PERF_PREFIX:-sudo -n}"
[[ "$T" == tcp || "$T" == udp ]] || { echo "STCP_TRANSPORT must be tcp or udp" >&2; exit 2; }
mkdir -p "$O"
V=(); [[ "${VERIFY:-0}" == 1 ]] && V=(--verify)
remote_irq_snapshot(){
  local output="$1"
  ssh $SSH_OPTS "$RPI_SSH" \
    "python3 '$RPI_BENCHMARK_DIR/irq_snapshot.py' --network-pattern $(printf '%q' "$IRQ_NETWORK_PATTERN")" \
    >"$output"
}


remote_perf_start(){
  local remote_output="$1"
  local remote_log="$2"
  local seconds="$3"
  local remote_pid_file="$4"

  ssh $SSH_OPTS "$RPI_SSH" "bash -s" -- \
    "$remote_output" "$remote_log" "$seconds" "$remote_pid_file" \
    "$PERF_EVENTS" "$REMOTE_PERF_PREFIX" <<'REMOTE_PERF'
set -u
output="$1"
log="$2"
seconds="$3"
pid_file="$4"
events="$5"
prefix="$6"

rm -f "$output" "$log" "$pid_file"

if ! command -v perf >/dev/null 2>&1; then
  echo "perf command not found" >"$log"
  exit 3
fi

# shellcheck disable=SC2086
nohup sh -c "$prefix perf stat -a -x ';' -o '$output' -e '$events' -- sleep '$seconds'" \
  >"$log" 2>&1 &
echo $! >"$pid_file"
REMOTE_PERF
}

remote_perf_finish(){
  local remote_output="$1"
  local remote_log="$2"
  local remote_pid_file="$3"
  local local_output="$4"
  local local_log="$5"

  ssh $SSH_OPTS "$RPI_SSH" "bash -s" -- \
    "$remote_output" "$remote_pid_file" <<'REMOTE_WAIT' >"$local_output"
set -u
output="$1"
pid_file="$2"

if [[ -f "$pid_file" ]]; then
  pid="$(cat "$pid_file" 2>/dev/null || true)"
  if [[ -n "$pid" ]]; then
    while kill -0 "$pid" 2>/dev/null; do sleep 0.1; done
  fi
fi

[[ -f "$output" ]] && cat "$output"
REMOTE_WAIT

  ssh $SSH_OPTS "$RPI_SSH" \
    "cat '$remote_log' 2>/dev/null || true" >"$local_log" || true
}

run_case(){
  local mode="$1" port="$2" clients="$3" payload="$4" pipeline="$5"
  local label="$mode"; [[ "$mode" == stcp ]] && label="stcp-$T"
  local name="${label}-c${clients}-p${payload}-q${pipeline}"
  local before="$O/$name.irq-before.json"
  local after="$O/$name.irq-after.json"
  local result="$O/$name.json"
  local perf_local="$O/$name.perf.csv"
  local perf_log_local="$O/$name.perf.log"
  local perf_remote="/tmp/stcp-perf-$USER-$name-$$.csv"
  local perf_log_remote="/tmp/stcp-perf-$USER-$name-$$.log"
  local perf_pid_remote="/tmp/stcp-perf-$USER-$name-$$.pid"
  local perf_seconds=$((DU + PERF_GRACE_SECONDS))
  local rc=0

  echo "===== $name ====="

  if [[ "$IRQ_METRICS" == 1 ]]; then
    remote_irq_snapshot "$before"
  fi

  if [[ "$PERF_METRICS" == 1 ]]; then
    if remote_perf_start         "$perf_remote" "$perf_log_remote" "$perf_seconds" "$perf_pid_remote"; then
      sleep 0.35
    else
      echo "[WARN] perf start failed for $name; continuing without perf metrics"
    fi
  fi

  set +e
  python3 "$D/benchmark_client.py" \
    --mode "$mode" --transport "$T" --host "$H" --port "$port" \
    --clients "$clients" --payload "$payload" --pipeline "$pipeline" \
    --duration "$DU" --output-json "$result" "${V[@]}"
  rc=$?
  set -e

  if [[ "$IRQ_METRICS" == 1 ]]; then
    remote_irq_snapshot "$after"
    if [[ -f "$result" ]]; then
      python3 "$D/enrich_irq_metrics.py" \
        --result "$result" --before "$before" --after "$after"
    fi
  fi

  return "$rc"
}
for payload in $PL; do
  for clients in $CL; do
    for pipeline in $Q; do
      run_case tcp "$TCP_PORT" "$clients" "$payload" "$pipeline"
      run_case tls "$TLS_PORT" "$clients" "$payload" "$pipeline"
      run_case stcp "$STCP_PORT" "$clients" "$payload" "$pipeline"
    done
  done
done
python3 "$D/generate_report.py" --input-dir "$O"
echo "Reports: $O/report.md $O/results.csv $O/summary.json"
