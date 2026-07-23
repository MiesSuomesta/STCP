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

STCP_OPERATION_TIMEOUT="${STCP_OPERATION_TIMEOUT:-30}"
CASE_GRACE_SECONDS="${CASE_GRACE_SECONDS:-20}"
CONTINUE_CASE_ON_ERROR="${CONTINUE_CASE_ON_ERROR:-1}"
PERF_STARTUP_WAIT_SECONDS="${PERF_STARTUP_WAIT_SECONDS:-0.6}"

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

  ssh $SSH_OPTS "$RPI_SSH" "bash -s -- $(printf '%q ' "$remote_output" "$remote_log" "$seconds" "$remote_pid_file" "$PERF_EVENTS" "$REMOTE_PERF_PREFIX" "$PERF_STARTUP_WAIT_SECONDS")" <<'REMOTE_PERF'
set -u
output="$1"
log="$2"
seconds="$3"
pid_file="$4"
events="$5"
prefix="$6"
startup_wait="$7"

rm -f "$output" "$log" "$pid_file"

perf_bin="$(command -v perf 2>/dev/null || true)"
if [[ -z "$perf_bin" ]]; then
  echo "perf command not found on $(hostname)" >"$log"
  exit 3
fi

# Run through a small wrapper so an optional prefix such as 'sudo -n' works.
# The wrapper records both perf diagnostics and its final exit status.
cmd="$prefix '$perf_bin' stat -a -x ';' -o '$output' -e '$events' -- sleep '$seconds'"
nohup bash -c "$cmd; rc=\$?; echo \$rc > '${pid_file}.status'; exit \$rc" \
  >"$log" 2>&1 &
pid=$!
echo "$pid" >"$pid_file"

sleep "$startup_wait"
if ! kill -0 "$pid" 2>/dev/null; then
  rc="$(cat "${pid_file}.status" 2>/dev/null || echo 1)"
  echo "perf exited during startup (status $rc)" >>"$log"
  exit "$rc"
fi
REMOTE_PERF
}
remote_perf_finish(){
  local remote_output="$1"
  local remote_log="$2"
  local remote_pid_file="$3"
  local local_output="$4"
  local local_log="$5"

  ssh $SSH_OPTS "$RPI_SSH" "bash -s -- $(printf '%q ' "$remote_output" "$remote_log" "$remote_pid_file")" <<'REMOTE_WAIT' >"$local_output"
set -u
output="$1"
log="$2"
pid_file="$3"

if [[ -f "$pid_file" ]]; then
  pid="$(cat "$pid_file" 2>/dev/null || true)"
  if [[ -n "$pid" ]]; then
    while kill -0 "$pid" 2>/dev/null; do sleep 0.1; done
  fi
fi

[[ -f "$output" ]] && cat "$output"
REMOTE_WAIT

  ssh $SSH_OPTS "$RPI_SSH" \
    "cat $(printf '%q' "$remote_log") 2>/dev/null || true; rm -f $(printf '%q' "$remote_output") $(printf '%q' "$remote_log") $(printf '%q' "$remote_pid_file") $(printf '%q' "${remote_pid_file}.status")" \
    >"$local_log" || true
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
  local perf_started=0
  local rc=0

  echo "===== $name ====="

  if [[ "$IRQ_METRICS" == 1 ]]; then
    remote_irq_snapshot "$before"
  fi

  if [[ "$PERF_METRICS" == 1 ]]; then
    if remote_perf_start         "$perf_remote" "$perf_log_remote" "$perf_seconds" "$perf_pid_remote"; then
      perf_started=1
    else
      ssh $SSH_OPTS "$RPI_SSH" "cat $(printf '%q' "$perf_log_remote") 2>/dev/null || true" >"$perf_log_local" || true
      echo "[WARN] perf start failed for $name; continuing without perf metrics"
      [[ -s "$perf_log_local" ]] && sed 's/^/[perf] /' "$perf_log_local"
    fi
  fi

  set +e
  timeout --signal=TERM --kill-after=5 "$((DU + STCP_OPERATION_TIMEOUT + CASE_GRACE_SECONDS))" \
    python3 "$D/benchmark_client.py" \
      --mode "$mode" --transport "$T" --host "$H" --port "$port" \
      --clients "$clients" --payload "$payload" --pipeline "$pipeline" \
      --duration "$DU" --timeout "$STCP_OPERATION_TIMEOUT" \
      --output-json "$result" "${V[@]}"
  rc=$?
  set -e
  if [[ "$rc" == 124 || "$rc" == 137 ]]; then
    echo "[TIMEOUT] $name exceeded the case deadline; continuing to the next case"
  fi

  if [[ "$IRQ_METRICS" == 1 ]]; then
    remote_irq_snapshot "$after"
    if [[ -f "$result" ]]; then
      python3 "$D/enrich_irq_metrics.py" \
        --result "$result" --before "$before" --after "$after"
    fi
  fi

  if [[ "$perf_started" == 1 ]]; then
    remote_perf_finish \
      "$perf_remote" "$perf_log_remote" "$perf_pid_remote" \
      "$perf_local" "$perf_log_local"
    if [[ -f "$result" && -s "$perf_local" ]]; then
      python3 "$D/enrich_perf_metrics.py" \
        --result "$result" --perf "$perf_local"
    else
      echo "[WARN] perf produced no counters for $name"
      [[ -s "$perf_log_local" ]] && sed 's/^/[perf] /' "$perf_log_local"
    fi
  fi

  if (( rc != 0 )); then
    echo "[FAIL] $name exited with status $rc"
    if [[ "$CONTINUE_CASE_ON_ERROR" == 1 ]]; then
      echo "[INFO] Continuing benchmark matrix after failed case"
      return 0
    fi
  fi
  return "$rc"
}

CASES=0

for payload in $PL; do
  for clients in $CL; do
    for pipeline in $Q; do
      CASES=$(( $CASES + 1))
    done
  done
done

CASE=0
for payload in $PL; do
  for clients in $CL; do
    for pipeline in $Q; do
      run_case tcp "$TCP_PORT" "$clients" "$payload" "$pipeline"
      run_case tls "$TLS_PORT" "$clients" "$payload" "$pipeline"
      run_case stcp "$STCP_PORT" "$clients" "$payload" "$pipeline"
      CASE=$(( $CASE + 1))
      echo "Cases done: $CASE / $CASES ..."
    done
  done
done

python3 "$D/generate_report.py" --input-dir "$O"
echo "Reports: $O/report.md $O/results.csv $O/summary.json"
