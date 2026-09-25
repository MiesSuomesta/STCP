#!/usr/bin/env bash
set -euo pipefail

if (($# != 10)); then
  echo "usage: $0 RUN_DIR CASE_ID MODE HOST PORT CLIENTS PAYLOAD PIPELINE DURATION DIRECTION" >&2
  exit 2
fi

D=$(cd "$(dirname "$0")" && pwd)
RUN_DIR=$1 CASE_ID=$2 MODE=$3 HOST=$4 PORT=$5 CLIENTS=$6 PAYLOAD=$7 PIPELINE=$8 DURATION=$9 DIRECTION=${10}
MAX_ATTEMPTS=${MAX_ATTEMPTS:-5}
CLIENT=${BENCHMARK_CLIENT:-$D/raspberrypi/benchmark_client.py}
STREAM_RUNNER=${STREAM_RUNNER:-$D/../run-stream-throughput.py}
STREAM_BENCH=${STREAM_BENCH:-$D/../tcp-path/stcp_tcp_path_bench}
STREAM_SOURCE=${STREAM_SOURCE:-$D/../tcp-path/stcp_tcp_path_bench.c}
STREAM_CHUNK=${STREAM_CHUNK:-128044}
STREAM_COUNT=${STREAM_COUNT:-256}
STREAM_ROUNDS=${STREAM_ROUNDS:-3}
RESTART_SERVERS=${RESTART_SERVERS:-:}

build_stream_bench() {
  if [[ ! -x "$STREAM_BENCH" || "$STREAM_SOURCE" -nt "$STREAM_BENCH" ]]; then
    echo "building stream benchmark: $STREAM_BENCH"
    cc -O2 -Wall -Wextra -o "$STREAM_BENCH" "$STREAM_SOURCE"
  fi
}
mkdir -p "$RUN_DIR/logs" "$RUN_DIR/attempts"

for ((attempt=1; attempt<=MAX_ATTEMPTS; attempt++)); do
  raw="$RUN_DIR/attempts/$CASE_ID.attempt-$attempt.json"
  log="$RUN_DIR/logs/$CASE_ID.attempt-$attempt.log"
  if ((attempt > 1)); then
    bash -c "$RESTART_SERVERS" >>"$log" 2>&1
  fi
  set +e
  if [[ "$DIRECTION" == "stream" && ( "$MODE" == "tcp" || "$MODE" == "stcp" ) ]]; then
    build_stream_bench >>"$log" 2>&1
    stream_json="$RUN_DIR/attempts/$CASE_ID.attempt-$attempt.stream.json"
    python3 "$STREAM_RUNNER" \
      --bench "$STREAM_BENCH" --transport "$MODE" --remote "$HOST" --port "$PORT" \
      --chunk "$STREAM_CHUNK" --count "$STREAM_COUNT" --rounds "$STREAM_ROUNDS" \
      --output "$stream_json" --benchctl-output "$raw" \
      --clients "$CLIENTS" --payload-bytes "$PAYLOAD" --pipeline "$PIPELINE" \
      >>"$log" 2>&1
    rc=$?
  else
    python3 "$CLIENT" --mode "$MODE" --host "$HOST" --port "$PORT" \
      --clients "$CLIENTS" --payload "$PAYLOAD" --pipeline "$PIPELINE" \
      --duration "$DURATION" --verify --output-json "$raw" >>"$log" 2>&1
    rc=$?
  fi
  set -e
  if ((rc == 0)) && python3 "$D/benchctl.py" record-case --raw "$raw" \
      --output "$RUN_DIR/cases/$CASE_ID.json" --case-id "$CASE_ID" \
      --direction "$DIRECTION" --attempt "$attempt" >>"$log" 2>&1; then
    if python3 - "$RUN_DIR/cases/$CASE_ID.json" <<'PY'
import json, sys
r=json.load(open(sys.argv[1]))
raise SystemExit(0 if r["errors"] == 0 and r["operations"] > 0 and not r["error_details"] else 1)
PY
    then exit 0; fi
  fi
done
echo "$CASE_ID failed after $MAX_ATTEMPTS attempts" >&2
exit 1
