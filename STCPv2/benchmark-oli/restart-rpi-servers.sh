#!/usr/bin/env bash
#
# Restart the exact benchmark_server.py processes currently active on Raspberry.
# Called automatically before every benchmark_client.py invocation.
#

set -Eeuo pipefail

RPI_ADDR="${RPI_ADDR:-192.168.1.199}"
RPI_USER="${RPI_USER:-pi}"
RPI_BENCHMARK_DIR="${RPI_BENCHMARK_DIR:-/home/pi/benchmark}"
SERVER_RESTART_DELAY="${SERVER_RESTART_DELAY:-2}"
SSH_OPTS=(
    -o BatchMode=yes
    -o ConnectTimeout=10
    -o StrictHostKeyChecking=accept-new
)

log()  { printf '[CASE-RESTART] %s\n' "$*"; }
die()  { printf '[CASE-RESTART][FAIL] %s\n' "$*" >&2; exit 1; }

[[ "$SERVER_RESTART_DELAY" =~ ^[0-9]+([.][0-9]+)?$ ]] ||
    die "SERVER_RESTART_DELAY must be numeric"

log "Restarting Raspberry benchmark servers"

ssh "${SSH_OPTS[@]}" "${RPI_USER}@${RPI_ADDR}" \
    "RPI_BENCHMARK_DIR='$RPI_BENCHMARK_DIR' SERVER_RESTART_DELAY='$SERVER_RESTART_DELAY' bash -s" <<'REMOTE'
set -Eeuo pipefail

cd "$RPI_BENCHMARK_DIR"

state_dir="logs/case-restart-state"
commands_file="$state_dir/commands.nul"
mkdir -p "$state_dir"
: >"$commands_file"

mapfile -t pids < <(
    pgrep -f '^python3 .*benchmark_server\.py' 2>/dev/null || true
)

if ((${#pids[@]} == 0)); then
    echo "[CASE-RESTART][FAIL] No active benchmark_server.py processes found" >&2
    exit 1
fi

for pid in "${pids[@]}"; do
    [[ -r "/proc/$pid/cmdline" ]] || continue

    cwd="$(readlink -f "/proc/$pid/cwd" 2>/dev/null || pwd)"
    printf '%s\0' "$cwd" >>"$commands_file"
    cat "/proc/$pid/cmdline" >>"$commands_file"
    printf '\0\0' >>"$commands_file"
done

kill "${pids[@]}" 2>/dev/null || true

for _ in $(seq 1 30); do
    remaining="$(pgrep -f '^python3 .*benchmark_server\.py' 2>/dev/null || true)"
    [[ -z "$remaining" ]] && break
    sleep 0.1
done

remaining="$(pgrep -f '^python3 .*benchmark_server\.py' 2>/dev/null || true)"
if [[ -n "$remaining" ]]; then
    kill -KILL $remaining 2>/dev/null || true
fi

python3 - "$commands_file" "$state_dir" <<'PY'
import os
import subprocess
import sys
from pathlib import Path

commands_file = Path(sys.argv[1])
state_dir = Path(sys.argv[2])
data = commands_file.read_bytes().split(b"\0")

i = 0
launched = 0
while i < len(data):
    while i < len(data) and not data[i]:
        i += 1
    if i >= len(data):
        break

    cwd = data[i].decode()
    i += 1
    argv = []

    while i < len(data) and data[i]:
        argv.append(data[i].decode())
        i += 1

    while i < len(data) and not data[i]:
        i += 1

    if not argv:
        continue

    name = f"server-{launched}"
    log_path = state_dir / f"{name}.log"
    pid_path = state_dir / f"{name}.pid"

    with log_path.open("ab", buffering=0) as log:
        proc = subprocess.Popen(
            argv,
            cwd=cwd,
            stdin=subprocess.DEVNULL,
            stdout=log,
            stderr=subprocess.STDOUT,
            start_new_session=True,
            close_fds=True,
        )

    pid_path.write_text(str(proc.pid) + "\n")
    launched += 1

if launched == 0:
    raise SystemExit("No benchmark server commands were saved")
PY

sleep "$SERVER_RESTART_DELAY"

mapfile -t new_pids < <(
    pgrep -f '^python3 .*benchmark_server\.py' 2>/dev/null || true
)

if ((${#new_pids[@]} == 0)); then
    echo "[CASE-RESTART][FAIL] Benchmark servers did not restart" >&2
    exit 1
fi

echo "[CASE-RESTART] Active benchmark servers:"
pgrep -af '^python3 .*benchmark_server\.py' || true
REMOTE

log "Raspberry benchmark servers restarted"
