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

STRESS_DURATION="${STRESS_DURATION:-30}"
THROUGHPUT_DURATION="${THROUGHPUT_DURATION:-$STRESS_DURATION}"
THROUGHPUT_CLIENTS="${THROUGHPUT_CLIENTS:-8}"
THROUGHPUT_PAYLOAD="${THROUGHPUT_PAYLOAD:-1048576}"
THROUGHPUT_PIPELINE="${THROUGHPUT_PIPELINE:-8}"
REPORT_EVERY="${REPORT_EVERY:-5}"

while (($#)); do
    case "$1" in
        --duration) STRESS_DURATION="$2"; THROUGHPUT_DURATION="$2"; shift 2 ;;
        --pipeline) THROUGHPUT_PIPELINE="$2"; shift 2 ;;
        --clients) THROUGHPUT_CLIENTS="$2"; shift 2 ;;
        --payload) THROUGHPUT_PAYLOAD="$2"; shift 2 ;;
        --report-every|--report-interval) REPORT_EVERY="$2"; shift 2 ;;
        --help|-h)
            echo "Usage: RUN_STRESS=1 bash run_stress_suite.sh [--duration SEC] [--pipeline N] [--clients N] [--payload BYTES] [--report-every SEC]"
            exit 0 ;;
        *) echo "Unknown argument: $1" >&2; exit 2 ;;
    esac
done

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
echo "=== STCP throughput: ${THROUGHPUT_CLIENTS} clients / ${THROUGHPUT_DURATION} s ==="
run_python ./stcp_stress.py \
    --mode throughput \
    --pipeline "$THROUGHPUT_PIPELINE" \
    --port 7780 \
    --clients "$THROUGHPUT_CLIENTS" \
    --payload "$THROUGHPUT_PAYLOAD" \
    --duration "$THROUGHPUT_DURATION" \
    --report-every "$REPORT_EVERY" \
    --no-verify \
    --json result-throughput-4.json

echo
echo "=== STCP mixed: 16 clients / 30 s ==="
run_python ./stcp_stress.py \
    --mode mixed \
    --port 7781 \
    --reconnect-every 128 \
    --clients 16 \
    --payload 262144 \
    --duration "$STRESS_DURATION" \
    --report-every "$REPORT_EVERY" \
    --no-verify \
    --json result-mixed-8.json

echo
echo "=== STCP churn: 16 clients / 30 s ==="
run_python ./stcp_stress.py \
    --mode churn \
    --port 7782 \
    --clients 16 \
    --payload 4096 \
    --duration "$STRESS_DURATION" \
    --report-every "$REPORT_EVERY" \
    --no-verify \
    --json result-churn-16.json
