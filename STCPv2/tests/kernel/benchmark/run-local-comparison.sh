#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
DURATION="${DURATION:-30}"
CLIENTS="${CLIENTS:-4}"
PAYLOAD="${PAYLOAD:-262144}"

[[ -f cert.pem && -f key.pem ]] || ./create-test-cert.sh

PIDS=()
cleanup() {
  for pid in "${PIDS[@]:-}"; do
    kill "$pid" 2>/dev/null || true
    wait "$pid" 2>/dev/null || true
  done
}
trap cleanup EXIT

run_one() {
  local mode="$1" port="$2"
  local extra_server=() extra_client=()
  local log="benchmark-${mode}-server.log"

  if [[ "$mode" == tls ]]; then
    extra_server=(--cert cert.pem --key key.pem)
    extra_client=(--insecure)
  fi

  rm -f "$log"
  PYTHONUNBUFFERED=1 python3 ./stcp_vs_tls.py server \
    --mode "$mode" --bind 127.0.0.1 --port "$port" \
    "${extra_server[@]}" >"$log" 2>&1 &
  local pid=$!
  PIDS+=("$pid")

  sleep 1
  if ! kill -0 "$pid" 2>/dev/null; then
    echo
    echo "===== $mode server failed ====="
    cat "$log"
    return 1
  fi

  cat "$log"
  echo
  echo "===== $mode ====="

  set +e
  python3 ./stcp_vs_tls.py client --mode "$mode" \
    --host 127.0.0.1 --port "$port" \
    --clients "$CLIENTS" --payload "$PAYLOAD" --duration "$DURATION" \
    --verbose "${extra_client[@]}"
  local rc=$?
  set -e

  kill "$pid" 2>/dev/null || true
  wait "$pid" 2>/dev/null || true

  if (( rc != 0 )); then
    echo
    echo "----- $mode server log -----"
    cat "$log"
    return "$rc"
  fi
}

run_one tcp 19000
run_one tls 19001

if [[ "${RUN_STCP:-0}" == 1 ]]; then
  run_one stcp 19002
else
  echo
  echo 'STCP skipped. Run all three with: RUN_STCP=1 ./run-local-comparison.sh'
fi
