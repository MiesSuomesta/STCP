#!/usr/bin/env bash
#
# STCP UDP benchmark orchestrator
#
# Orchestrates a complete client/server benchmark cycle:
#   1. synchronizes benchmark server files to Raspberry Pi
#   2. stops stale benchmark servers
#   3. starts fresh servers on Raspberry Pi
#   4. clears dmesg on both hosts before each test
#   5. runs the requested client matrix on the local development host
#   6. saves client output, local/remote dmesg and remote server logs
#   7. creates a compressed archive of the complete run
#
# Run from the kernel-module project root:
#
#   HOST=192.168.1.199 \
#   PORT=19002 \
#   CLIENTS_LIST="16 8 4 2 1" \
#   bash benchmark/orchestrate-stcp-udp-tests.sh
#
# Useful overrides:
#
#   RPI_USER=pi
#   RPI_HOST=192.168.1.199
#   RPI_BENCH_DIR=/home/pi/stcp-benchmark
#   TEST_SCRIPT=benchmark/test-stcp-udp-direct-tx-burst-v13.sh
#   TEST_NAME=stcp-udp-baseline
#   CONTROL_BUDGET=64
#   RESTART_SERVERS_EACH_RUN=1
#   CLEAR_DMESG=1
#   CONTINUE_ON_FAILURE=1
#

set -uo pipefail

PROJECT_ROOT="${PROJECT_ROOT:-$(pwd)}"
BENCH_DIR="${BENCH_DIR:-$PROJECT_ROOT/benchmark}"

RPI_USER="${RPI_USER:-pi}"
RPI_HOST="${RPI_HOST:-${HOST:-192.168.1.199}}"
RPI_SSH_PORT="${RPI_SSH_PORT:-22}"
RPI_TARGET="${RPI_USER}@${RPI_HOST}"
RPI_BENCH_DIR="${RPI_BENCH_DIR:-/home/${RPI_USER}/stcp-benchmark}"

HOST="${HOST:-$RPI_HOST}"
PORT="${PORT:-19002}"
CLIENTS_LIST="${CLIENTS_LIST:-16 8 4 2 1}"

TEST_SCRIPT="${TEST_SCRIPT:-benchmark/test-stcp-udp-direct-tx-burst-v13.sh}"
TEST_NAME="${TEST_NAME:-stcp-udp-orchestrated}"
CONTROL_BUDGET="${CONTROL_BUDGET:-}"

RESTART_SERVERS_EACH_RUN="${RESTART_SERVERS_EACH_RUN:-1}"
CLEAR_DMESG="${CLEAR_DMESG:-1}"
CONTINUE_ON_FAILURE="${CONTINUE_ON_FAILURE:-1}"
SYNC_BENCHMARKS="${SYNC_BENCHMARKS:-1}"
KEEP_REMOTE_RUNNING="${KEEP_REMOTE_RUNNING:-0}"

TIMESTAMP="${TIMESTAMP:-$(date +%Y%m%d-%H%M%S)}"
RESULT_ROOT="${RESULT_ROOT:-$BENCH_DIR/results/orchestrated-${TIMESTAMP}}"
ARCHIVE="${ARCHIVE:-${RESULT_ROOT}.tar.gz}"

SSH_OPTS=(
    -p "$RPI_SSH_PORT"
    -o ConnectTimeout=10
    -o ServerAliveInterval=15
    -o ServerAliveCountMax=4
)

log() {
    printf '[%s] %s\n' "$(date '+%F %T')" "$*"
}

die() {
    log "ERROR: $*"
    exit 1
}

remote() {
    ssh "${SSH_OPTS[@]}" "$RPI_TARGET" "$@"
}

remote_tty() {
    ssh -tt "${SSH_OPTS[@]}" "$RPI_TARGET" "$@"
}

safe_name() {
    printf '%s' "$1" | tr -cs 'A-Za-z0-9._-' '-'
}

require_file() {
    [[ -f "$1" ]] || die "Required file does not exist: $1"
}

require_command() {
    command -v "$1" >/dev/null 2>&1 || die "Required command not found: $1"
}

cleanup_on_exit() {
    local rc=$?

    if [[ "$KEEP_REMOTE_RUNNING" != "1" ]]; then
        log "Stopping Raspberry benchmark servers"
        remote "
            set +e
            if [[ -x '$RPI_BENCH_DIR/stop-servers.sh' ]]; then
                cd '$RPI_BENCH_DIR'
                ./stop-servers.sh
            else
                pkill -f '[b]enchmark_server.py'
            fi
            exit 0
        " >/dev/null 2>&1 || true
    fi

    if [[ -d "$RESULT_ROOT" ]]; then
        {
            echo "exit_status=$rc"
            echo "finished_at=$(date --iso-8601=seconds)"
        } >>"$RESULT_ROOT/run-metadata.txt"
    fi

    exit "$rc"
}

trap cleanup_on_exit EXIT
trap 'log "Interrupted"; exit 130' INT TERM

cd "$PROJECT_ROOT"

require_command ssh
require_command tar
require_command python3
require_command sudo
require_file "$TEST_SCRIPT"
[[ -d "$BENCH_DIR" ]] || die "Benchmark directory does not exist: $BENCH_DIR"

mkdir -p "$RESULT_ROOT"/{runs,server-logs,system}

log "Validating local sudo access"
sudo -v

log "Checking SSH access to $RPI_TARGET"
remote "printf 'connected\n'" >/dev/null

{
    echo "test_name=$TEST_NAME"
    echo "timestamp=$TIMESTAMP"
    echo "project_root=$PROJECT_ROOT"
    echo "test_script=$TEST_SCRIPT"
    echo "host=$HOST"
    echo "port=$PORT"
    echo "clients_list=$CLIENTS_LIST"
    echo "rpi_target=$RPI_TARGET"
    echo "rpi_bench_dir=$RPI_BENCH_DIR"
    echo "restart_servers_each_run=$RESTART_SERVERS_EACH_RUN"
    echo "clear_dmesg=$CLEAR_DMESG"
    echo "git_commit=$(git rev-parse HEAD 2>/dev/null || echo unknown)"
    echo "git_description=$(git describe --always --dirty 2>/dev/null || echo unknown)"
    echo "started_at=$(date --iso-8601=seconds)"
} >"$RESULT_ROOT/run-metadata.txt"

log "Collecting initial system information"

{
    echo "=== local ==="
    date --iso-8601=seconds
    uname -a
    echo
    git status --short 2>/dev/null || true
    echo
    git log -1 --oneline 2>/dev/null || true
    echo
    if [[ -r /sys/module/stcp/parameters/udp_sendmsg_frames ]]; then
        printf 'udp_sendmsg_frames='
        cat /sys/module/stcp/parameters/udp_sendmsg_frames
    fi
    if [[ -r /sys/module/stcp/parameters/udp_rx_control_budget ]]; then
        printf 'udp_rx_control_budget='
        cat /sys/module/stcp/parameters/udp_rx_control_budget
    fi
} >"$RESULT_ROOT/system/local.txt" 2>&1

remote "
    set +e
    echo '=== raspberry ==='
    date --iso-8601=seconds
    uname -a
    echo
    lsmod | grep '^stcp' || true
    echo
    modinfo stcp 2>/dev/null | grep -E '^(filename|version|vermagic):' || true
    echo
    if [[ -r /sys/module/stcp/parameters/udp_sendmsg_frames ]]; then
        printf 'udp_sendmsg_frames='
        cat /sys/module/stcp/parameters/udp_sendmsg_frames
    fi
    if [[ -r /sys/module/stcp/parameters/udp_rx_control_budget ]]; then
        printf 'udp_rx_control_budget='
        cat /sys/module/stcp/parameters/udp_rx_control_budget
    fi
" >"$RESULT_ROOT/system/raspberry.txt" 2>&1

if [[ "$SYNC_BENCHMARKS" == "1" ]]; then
    log "Synchronizing benchmark server files to Raspberry"

    # --touch during extraction avoids misleading "timestamp is in the future"
    # warnings when the two machines' clocks differ slightly.
    tar \
        --exclude='results' \
        --exclude='__pycache__' \
        --exclude='*.pyc' \
        --exclude='.pytest_cache' \
        -C "$BENCH_DIR" \
        -czf - . |
    remote "
        set -e
        mkdir -p '$RPI_BENCH_DIR'
        tar --touch -xzf - -C '$RPI_BENCH_DIR'
        chmod +x '$RPI_BENCH_DIR'/*.sh 2>/dev/null || true
    "
fi

stop_servers() {
    log "Stopping stale Raspberry benchmark servers"

    remote "
        set +e
        cd '$RPI_BENCH_DIR' 2>/dev/null || true

        if [[ -x ./stop-servers.sh ]]; then
            ./stop-servers.sh >/dev/null 2>&1 || true
        fi

        pids=\$(pgrep -f '^python3 .*benchmark_server\.py' || true)
        if [[ -n \"\$pids\" ]]; then
            kill \$pids 2>/dev/null || true
            sleep 1
        fi

        pids=\$(pgrep -f '^python3 .*benchmark_server\.py' || true)
        if [[ -n \"\$pids\" ]]; then
            kill -KILL \$pids 2>/dev/null || true
            sleep 1
        fi

        remaining=\$(pgrep -af '^python3 .*benchmark_server\.py' || true)
        if [[ -n \"\$remaining\" ]]; then
            echo 'ERROR: benchmark_server.py processes are still running:' >&2
            echo \"\$remaining\" >&2
            exit 1
        fi

        exit 0
    "
}

start_servers() {
    local run_label="$1"
    local process_log="$RESULT_ROOT/server-logs/processes-${run_label}.txt"
    local carrier_log="$RESULT_ROOT/server-logs/carrier-${run_label}.txt"

    log "Starting Raspberry benchmark servers for $run_label"

    remote "
        set +e
        cd '$RPI_BENCH_DIR' || exit 1
        mkdir -p logs

        # Start the project's normal server set first so TCP/TLS/raw UDP
        # retain their existing certificate and command-line handling.
        if [[ -x ./start-servers.sh ]]; then
            ./start-servers.sh >logs/start-servers-${run_label}.log 2>&1
        else
            echo 'ERROR: start-servers.sh is missing or not executable' >&2
            exit 1
        fi

        sleep 1

        # The ba2c943-era helper may start port 19002 with --transport tcp.
        # Stop only STCP benchmark_server.py instances and restart exactly one
        # STCP/UDP server. Do not use broad pkill patterns that can kill SSH.
        stcp_pids=\$(pgrep -f '^python3 .*benchmark_server\.py .*--mode stcp( |$)' || true)
        if [[ -n \"\$stcp_pids\" ]]; then
            kill \$stcp_pids 2>/dev/null || true
            sleep 1
        fi

        nohup python3 ./benchmark_server.py \
            --mode stcp \
            --transport udp \
            --host 0.0.0.0 \
            --port '$PORT' \
            >logs/stcp-udp-${run_label}.log 2>&1 &

        echo \$! >logs/stcp-udp-${run_label}.pid
        sleep 2

        echo '=== benchmark processes ==='
        pgrep -af '^python3 .*benchmark_server\.py' || true

        echo
        echo '=== port 19002 ==='
        ss -lunp 2>/dev/null | grep ':$PORT ' || true

        # Hard validation: refuse to benchmark if port 19002 is not served by
        # an explicitly UDP-configured STCP process.
        if ! pgrep -af '^python3 .*benchmark_server\.py .*--mode stcp .*--transport udp .*--port $PORT( |$)' >/dev/null; then
            echo 'ERROR: STCP server on port $PORT is not using UDP transport' >&2
            echo 'start-server output:' >&2
            tail -200 logs/start-servers-${run_label}.log >&2 || true
            echo 'STCP/UDP output:' >&2
            tail -200 logs/stcp-udp-${run_label}.log >&2 || true
            exit 1
        fi

        exit 0
    " >"$process_log" 2>&1
    local rc=$?

    if (( rc != 0 )); then
        log "ERROR: Raspberry server startup/validation failed for $run_label"
        cat "$process_log" >&2 || true
        return "$rc"
    fi

    # Preserve carrier creation/listen lines. dmesg is cleared before this
    # function, so these lines belong to the current case.
    remote_tty "
        set +e
        sudo dmesg --color=never |
            grep -E 'carrier: create complete.*kind=(udp|tcp)|listen complete.*kind=(udp|tcp)|RX thread enter.*kind=2' |
            tail -80
        exit 0
    " >"$carrier_log" 2>&1 || true

    if grep -q 'kind=tcp.*19002\|transport tcp' "$carrier_log" 2>/dev/null; then
        log "ERROR: Raspberry dmesg suggests TCP carrier on STCP port"
        return 1
    fi

    return 0
}

clear_logs() {
    local run_dir="$1"

    if [[ "$CLEAR_DMESG" != "1" ]]; then
        return 0
    fi

    log "Clearing local and Raspberry dmesg"

    sudo dmesg -C
    remote_tty "sudo dmesg -C" >"$run_dir/remote-dmesg-clear.txt" 2>&1
}

collect_logs() {
    local run_dir="$1"
    local run_label="$2"

    log "Collecting dmesg and server logs for $run_label"

    sudo dmesg --color=never >"$run_dir/dmesg-devaus.log" 2>&1 || true

    remote_tty "sudo dmesg --color=never" \
        >"$run_dir/dmesg-raspberry.log" 2>&1 || true

    remote "
        set +e
        cd '$RPI_BENCH_DIR'

        echo '=== processes ==='
        pgrep -af '[b]enchmark_server.py' || true

        echo
        echo '=== listening sockets ==='
        ss -lntup 2>/dev/null |
            grep -E ':(19000|19001|19002|19003)\b' || true

        echo
        echo '=== server logs ==='
        for file in \
            logs/tcp.log \
            logs/tls.log \
            logs/udp.log \
            logs/stcp.log \
            logs/stcp-tcp.log \
            logs/stcp-udp-${run_label}.log \
            logs/start-servers-${run_label}.log
        do
            [[ -f \"\$file\" ]] || continue
            echo
            echo \"===== \$file =====\"
            tail -500 \"\$file\"
        done
    " >"$run_dir/raspberry-server-state.log" 2>&1 || true

    {
        echo "=== local STCP summary ==="
        grep -E \
            'connect handshake|event=(301|302|306|308|309|314|315|316|320|321|322|323|324|325|326|327)|UDP carrier final stats' \
            "$run_dir/dmesg-devaus.log" || true
    } >"$run_dir/dmesg-devaus-summary.log"

    {
        echo "=== Raspberry STCP summary ==="
        grep -E \
            'ACCEPT COMPLETE|connect handshake|event=(301|302|306|308|309|314|315|316|320|321|322|323|324|325|326|327)|UDP carrier final stats' \
            "$run_dir/dmesg-raspberry.log" || true
    } >"$run_dir/dmesg-raspberry-summary.log"
}

run_case() {
    local clients="$1"
    local run_number="$2"
    local label
    local run_dir
    local status=0

    label="$(safe_name "run-${run_number}-c${clients}")"
    run_dir="$RESULT_ROOT/runs/$label"
    mkdir -p "$run_dir"

    if [[ "$RESTART_SERVERS_EACH_RUN" == "1" ]]; then
        stop_servers
        clear_logs "$run_dir"
        start_servers "$label" || return $?
    else
        clear_logs "$run_dir"
    fi

    {
        echo "run_number=$run_number"
        echo "clients=$clients"
        echo "host=$HOST"
        echo "port=$PORT"
        echo "test_script=$TEST_SCRIPT"
        echo "started_at=$(date --iso-8601=seconds)"
    } >"$run_dir/metadata.txt"

    log "Running $TEST_NAME: clients=$clients"

    set +e
    env \
        HOST="$HOST" \
        PORT="$PORT" \
        CLIENTS_LIST="$clients" \
        ${CONTROL_BUDGET:+CONTROL_BUDGET="$CONTROL_BUDGET"} \
        bash "$TEST_SCRIPT" \
        > >(tee "$run_dir/client-output.log") \
        2> >(tee "$run_dir/client-error.log" >&2)

    status=$?
    set -e

    {
        echo "exit_status=$status"
        echo "finished_at=$(date --iso-8601=seconds)"
    } >>"$run_dir/metadata.txt"

    collect_logs "$run_dir" "$label"

    if [[ "$status" -ne 0 ]]; then
        log "Case clients=$clients failed with status $status"

        if [[ "$CONTINUE_ON_FAILURE" != "1" ]]; then
            return "$status"
        fi
    else
        log "Case clients=$clients completed successfully"
    fi

    return 0
}

stop_servers

run_number=0
overall_status=0

for clients in $CLIENTS_LIST; do
    run_number=$((run_number + 1))

    if ! run_case "$clients" "$run_number"; then
        overall_status=$?
        break
    fi
done

log "Collecting final remote state"

remote "
    set +e
    date --iso-8601=seconds
    uname -a
    echo
    pgrep -af '[b]enchmark_server.py' || true
    echo
    df -h / /boot 2>/dev/null || true
" >"$RESULT_ROOT/system/raspberry-final.txt" 2>&1 || true

log "Creating archive: $ARCHIVE"
tar -C "$(dirname "$RESULT_ROOT")" \
    -czf "$ARCHIVE" \
    "$(basename "$RESULT_ROOT")"

log "Test setup complete"
log "Results: $RESULT_ROOT"
log "Archive: $ARCHIVE"

exit "$overall_status"
