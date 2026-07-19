#!/bin/bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PYTHON="${PYTHON:-python3}"
CHILD_PID=""
CHILD_PGID=""

cleanup() {
    local rc=$?
    trap - EXIT INT TERM
    if [[ -n "$CHILD_PID" ]] && kill -0 "$CHILD_PID" 2>/dev/null; then
        # The smoke test creates its own server process. Kill the complete
        # process group so a blocked STCP syscall cannot leave an orphan.
        kill -TERM -- "-${CHILD_PGID:-$CHILD_PID}" 2>/dev/null || true
        for _ in {1..20}; do
            kill -0 "$CHILD_PID" 2>/dev/null || break
            sleep 0.05
        done
        kill -KILL -- "-${CHILD_PGID:-$CHILD_PID}" 2>/dev/null || true
        # A task sleeping in uninterruptible kernel state (D) cannot be
        # reaped until the syscall returns. Never wait for it here: detach the
        # shell job so the runner itself can always exit.
        disown "$CHILD_PID" 2>/dev/null || true
    fi
    exit "$rc"
}
trap cleanup EXIT INT TERM

run_python() {
    local hard_timeout="${RUN_HARD_TIMEOUT:-0}"
    local started now rc=0

    # A custom kernel-family connect()/close() may sleep inside the kernel and
    # never return to Python. Run each tool in a new process group so the
    # watchdog can terminate both the client and every server child.
    setsid "$PYTHON" "$@" &
    CHILD_PID=$!
    CHILD_PGID=$CHILD_PID
    started=$(date +%s)

    set +e
    while kill -0 "$CHILD_PID" 2>/dev/null; do
        if (( hard_timeout > 0 )); then
            now=$(date +%s)
            if (( now - started >= hard_timeout )); then
                echo >&2
                echo "ERROR: test exceeded hard timeout (${hard_timeout}s)." >&2
                echo "A blocking STCP kernel syscall did not return; terminating process group ${CHILD_PGID}." >&2
                kill -TERM -- "-$CHILD_PGID" 2>/dev/null || true
                sleep 0.5
                kill -KILL -- "-$CHILD_PGID" 2>/dev/null || true
                # Do not call wait(1): a process stuck in TASK_UNINTERRUPTIBLE
                # remains unreapable even after SIGKILL and would hang this
                # script forever. Detach it and return the timeout status.
                disown "$CHILD_PID" 2>/dev/null || true
                rc=124
                break
            fi
        fi
        sleep 0.1
    done

    if (( rc == 0 )); then
        wait "$CHILD_PID"
        rc=$?
    fi
    set -e
    CHILD_PID=""
    CHILD_PGID=""
    return "$rc"
}

cd "$SCRIPT_DIR"

# Always establish basic correctness before any stress or throughput run.
RUN_HARD_TIMEOUT="${STCP_SMOKE_HARD_TIMEOUT:-20}" run_python ./stcp_smoke.py \
    --port "${STCP_SMOKE_PORT:-7777}" \
    --sizes "${STCP_SMOKE_SIZES:-64,4096,65536}" \
    --timeout "${STCP_SMOKE_TIMEOUT:-5}"

if [[ "${RUN_STRESS:-0}" != "1" ]]; then
    echo
    echo "Functional test passed. Stress tests were skipped."
    echo "Run the full suite with: RUN_STRESS=1 bash run_stress_suite.sh"
    exit 0
fi

echo
echo "=== STCP throughput: 4 clients / 25 s ==="
run_python ./stcp_stress.py \
    --mode throughput \
    --pipeline 1 \
    --port 7780 \
    --clients 4 \
    --payload 262144 \
    --duration 25 \
    --json result-throughput-4.json

echo
echo "=== STCP mixed: 8 clients / 30 s ==="
run_python ./stcp_stress.py \
    --mode mixed \
    --port 7781 \
    --reconnect-every 128 \
    --clients 8 \
    --payload 262144 \
    --duration 30 \
    --json result-mixed-8.json

echo
echo "=== STCP churn: 16 clients / 30 s ==="
run_python ./stcp_stress.py \
    --mode churn \
    --port 7782 \
    --clients 16 \
    --payload 4096 \
    --duration 30 \
    --json result-churn-16.json
