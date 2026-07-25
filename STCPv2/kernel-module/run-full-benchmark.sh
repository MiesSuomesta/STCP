#!/usr/bin/env bash
#
# STCP full build + deploy + benchmark pipeline
#
# Run from:
#   ~/git/STCP/STCPv2/kernel-module
#
# Example:
#   HOST=192.168.1.199 \
#   CLIENTS_LIST="16 8 4 2 1" \
#   bash run-full-benchmark.sh
#
# Important overrides:
#   RPI_USER=pi
#   RPI_HOST=192.168.1.199
#   RPI_SSH_PORT=22
#   JOBS=4
#   CLIENTS_LIST="16 8 4 2 1"
#   TEST_SCRIPT=benchmark/test-stcp-udp-direct-tx-burst-v13.sh
#   SKIP_X86_BUILD=0
#   SKIP_RPI_BUILD=0
#   SKIP_DEPLOY=0
#   SKIP_REBOOT=0
#   SKIP_BENCHMARK=0
#   KEEP_REMOTE_RUNNING=0
#
# The script intentionally avoids `set -e`.
# Every critical stage is checked explicitly, while expected non-zero statuses
# from grep/pkill/test commands do not terminate the whole pipeline.
#

set -uo pipefail

PROJECT_ROOT="${PROJECT_ROOT:-$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)}"
cd "$PROJECT_ROOT" || exit 1

RPI_USER="${RPI_USER:-pi}"
RPI_HOST="${RPI_HOST:-${HOST:-192.168.1.199}}"
RPI_SSH_PORT="${RPI_SSH_PORT:-22}"
RPI_TARGET="${RPI_USER}@${RPI_HOST}"

HOST="${HOST:-$RPI_HOST}"
PORT="${PORT:-19002}"
JOBS="${JOBS:-4}"
CLIENTS_LIST="${CLIENTS_LIST:-16 8 4 2 1}"
TEST_SCRIPT="${TEST_SCRIPT:-benchmark/test-stcp-udp-direct-tx-burst-v13.sh}"

SKIP_X86_BUILD="${SKIP_X86_BUILD:-0}"
SKIP_RPI_BUILD="${SKIP_RPI_BUILD:-0}"
SKIP_DEPLOY="${SKIP_DEPLOY:-0}"
SKIP_REBOOT="${SKIP_REBOOT:-0}"
SKIP_BENCHMARK="${SKIP_BENCHMARK:-0}"
KEEP_REMOTE_RUNNING="${KEEP_REMOTE_RUNNING:-0}"

TIMESTAMP="${TIMESTAMP:-$(date +%Y%m%d-%H%M%S)}"
RUN_NAME="${RUN_NAME:-full-${TIMESTAMP}}"
ARTIFACT_ROOT="${ARTIFACT_ROOT:-$PROJECT_ROOT/benchmark/results/$RUN_NAME}"
LOG_DIR="$ARTIFACT_ROOT/logs"
BUILD_DIR="$ARTIFACT_ROOT/build"
SYSTEM_DIR="$ARTIFACT_ROOT/system"
PACKAGE_DIR="$ARTIFACT_ROOT/packages"
FINAL_ARCHIVE="${FINAL_ARCHIVE:-${ARTIFACT_ROOT}.tar.gz}"

X86_DIR="${X86_DIR:-$PROJECT_ROOT/x86-kernel-module}"
RPI_DIR="${RPI_DIR:-$PROJECT_ROOT/raspberry-kernel-module}"
COMMON_RUST_DIR="${COMMON_RUST_DIR:-$PROJECT_ROOT/common-rust}"
BENCH_DIR="${BENCH_DIR:-$PROJECT_ROOT/benchmark}"

REBUILD_SCRIPT="${REBUILD_SCRIPT:-$PROJECT_ROOT/rebuild-all.sh}"
ORCHESTRATOR="${ORCHESTRATOR:-$PROJECT_ROOT/orchestrate-stcp-udp-tests-fixed.sh}"

SSH_OPTS=(
    -p "$RPI_SSH_PORT"
    -o ConnectTimeout=10
    -o ServerAliveInterval=15
    -o ServerAliveCountMax=4
)

mkdir -p "$LOG_DIR" "$BUILD_DIR" "$SYSTEM_DIR" "$PACKAGE_DIR"

log() {
    printf '[%s] %s\n' "$(date '+%F %T')" "$*" | tee -a "$LOG_DIR/pipeline.log"
}

fail() {
    log "ERROR: $*"
    return 1
}

run_logged() {
    local name="$1"
    shift

    log "START: $name"
    "$@" > >(tee "$LOG_DIR/${name}.out.log") \
         2> >(tee "$LOG_DIR/${name}.err.log" >&2)
    local rc=$?

    echo "$rc" >"$LOG_DIR/${name}.status"

    if (( rc != 0 )); then
        log "FAILED: $name (status=$rc)"
        return "$rc"
    fi

    log "OK: $name"
    return 0
}

remote() {
    ssh "${SSH_OPTS[@]}" "$RPI_TARGET" "$@"
}

remote_tty() {
    ssh -tt "${SSH_OPTS[@]}" "$RPI_TARGET" "$@"
}

collect_local_system_info() {
    {
        echo "timestamp=$(date --iso-8601=seconds)"
        echo "project_root=$PROJECT_ROOT"
        echo "git_commit=$(git rev-parse HEAD 2>/dev/null || echo unknown)"
        echo "git_description=$(git describe --always --dirty 2>/dev/null || echo unknown)"
        echo
        uname -a
        echo
        git status --short 2>/dev/null || true
        echo
        git log -5 --oneline 2>/dev/null || true
        echo
        df -h / /boot 2>/dev/null || true
        echo
        free -h 2>/dev/null || true
        echo
        lscpu 2>/dev/null || true
    } >"$SYSTEM_DIR/devaus-before.txt" 2>&1
}

collect_remote_system_info() {
    remote "
        set +e
        echo timestamp=\$(date --iso-8601=seconds)
        uname -a
        echo
        lsmod | grep '^stcp' || true
        echo
        modinfo stcp 2>/dev/null | grep -E '^(filename|version|vermagic):' || true
        echo
        df -h / /boot 2>/dev/null || true
        echo
        free -h 2>/dev/null || true
        echo
        vcgencmd get_throttled 2>/dev/null || true
        echo
        dmesg | grep -iE 'undervoltage|throttl' | tail -50 || true
    " >"$SYSTEM_DIR/raspberry-before.txt" 2>&1
}

check_requirements() {
    local missing=0
    local cmd

    for cmd in bash ssh scp tar git sudo make python3; do
        if ! command -v "$cmd" >/dev/null 2>&1; then
            log "Missing command: $cmd"
            missing=1
        fi
    done

    if (( missing != 0 )); then
        return 1
    fi

    if [[ ! -d "$X86_DIR" ]]; then
        fail "Missing x86 module directory: $X86_DIR"
        return 1
    fi

    if [[ ! -d "$RPI_DIR" ]]; then
        fail "Missing Raspberry module directory: $RPI_DIR"
        return 1
    fi

    if [[ ! -f "$TEST_SCRIPT" ]]; then
        fail "Missing benchmark test script: $TEST_SCRIPT"
        return 1
    fi

    return 0
}

build_x86() {
    if [[ "$SKIP_X86_BUILD" == "1" ]]; then
        log "Skipping x86 build"
        return 0
    fi

    if [[ -x "$REBUILD_SCRIPT" ]]; then
        log "Using project rebuild script for x86/RPi builds: $REBUILD_SCRIPT"
        run_logged "rebuild-all" env JOBS="$JOBS" bash "$REBUILD_SCRIPT"
        return $?
    fi

    log "No rebuild-all.sh found; using module Makefiles"

    run_logged "x86-clean" make -C "$X86_DIR" clean || true
    run_logged "x86-build" make -C "$X86_DIR" -j"$JOBS" module
}

build_rpi() {
    if [[ "$SKIP_RPI_BUILD" == "1" ]]; then
        log "Skipping Raspberry build"
        return 0
    fi

    # If rebuild-all.sh was already used in build_x86(), avoid rebuilding twice.
    if [[ -x "$REBUILD_SCRIPT" && "$SKIP_X86_BUILD" != "1" ]]; then
        return 0
    fi

    run_logged "rpi-clean" make -C "$RPI_DIR" clean || true
    run_logged "rpi-build" make -C "$RPI_DIR" -j"$JOBS" module
}

verify_build_outputs() {
    local failed=0

    if [[ "$SKIP_X86_BUILD" != "1" && ! -f "$X86_DIR/stcp.ko" ]]; then
        log "Missing x86 module: $X86_DIR/stcp.ko"
        failed=1
    fi

    if [[ "$SKIP_RPI_BUILD" != "1" && ! -f "$RPI_DIR/stcp.ko" ]]; then
        log "Missing Raspberry module: $RPI_DIR/stcp.ko"
        failed=1
    fi

    {
        echo "=== x86 ==="
        if [[ -f "$X86_DIR/stcp.ko" ]]; then
            modinfo "$X86_DIR/stcp.ko" 2>/dev/null || true
            sha256sum "$X86_DIR/stcp.ko"
        fi
        echo
        echo "=== Raspberry ==="
        if [[ -f "$RPI_DIR/stcp.ko" ]]; then
            modinfo "$RPI_DIR/stcp.ko" 2>/dev/null || true
            sha256sum "$RPI_DIR/stcp.ko"
        fi
    } >"$BUILD_DIR/module-info.txt" 2>&1

    return "$failed"
}

deploy_rpi() {
    if [[ "$SKIP_DEPLOY" == "1" ]]; then
        log "Skipping Raspberry deployment"
        return 0
    fi

    # Prefer the project's own deployment/package script when available.
    local candidate
    for candidate in \
        "$PROJECT_ROOT/build-rpi-package.sh" \
        "$PROJECT_ROOT/deploy-rpi.sh" \
        "$RPI_DIR/build-rpi-package.sh" \
        "$RPI_DIR/deploy-rpi.sh"
    do
        if [[ -x "$candidate" ]]; then
            log "Using deployment script: $candidate"
            run_logged "deploy-rpi" env \
                RPI_HOST="$RPI_TARGET" \
                HOST="$RPI_HOST" \
                JOBS="$JOBS" \
                bash "$candidate"
            return $?
        fi
    done

    log "No deployment helper found; copying stcp.ko directly"

    if [[ ! -f "$RPI_DIR/stcp.ko" ]]; then
        fail "Cannot deploy; Raspberry stcp.ko missing"
        return 1
    fi

    run_logged "scp-rpi-module" \
        scp "${SSH_OPTS[@]}" "$RPI_DIR/stcp.ko" "$RPI_TARGET:/tmp/stcp.ko" || return $?

    remote_tty "
        set +e
        sudo modprobe -r stcp 2>/dev/null || true
        sudo install -D -m 0644 /tmp/stcp.ko /lib/modules/\$(uname -r)/extra/stcp.ko
        rc=\$?
        if (( rc != 0 )); then
            exit \$rc
        fi
        sudo depmod -a
        sudo modprobe stcp
    " >"$LOG_DIR/deploy-rpi-manual.out.log" 2>"$LOG_DIR/deploy-rpi-manual.err.log"
    local rc=$?
    echo "$rc" >"$LOG_DIR/deploy-rpi-manual.status"
    return "$rc"
}

reboot_rpi_if_requested() {
    if [[ "$SKIP_REBOOT" == "1" ]]; then
        log "Skipping Raspberry reboot"
        return 0
    fi

    # Reboot only when the project deployment script likely installed a kernel.
    if [[ ! -x "$REBUILD_SCRIPT" && "$SKIP_DEPLOY" == "1" ]]; then
        log "No kernel deployment detected; not rebooting Raspberry"
        return 0
    fi

    log "Rebooting Raspberry"

    remote_tty "sudo reboot" >/dev/null 2>&1 || true

    local attempt
    for attempt in $(seq 1 90); do
        sleep 2
        if remote "echo online" >/dev/null 2>&1; then
            log "Raspberry is online after reboot"
            return 0
        fi
    done

    fail "Raspberry did not return after reboot"
    return 1
}

verify_remote_runtime() {
    log "Verifying Raspberry runtime"

    remote_tty "
        set +e

        echo '=== uname ==='
        uname -a

        echo
        echo '=== module ==='
        lsmod | grep '^stcp' || true
        modinfo stcp 2>/dev/null | grep -E '^(filename|version|vermagic):' || true

        echo
        echo '=== parameters ==='
        for p in /sys/module/stcp/parameters/*; do
            [[ -r \"\$p\" ]] || continue
            printf '%s=' \"\$(basename \"\$p\")\"
            cat \"\$p\"
        done

        echo
        echo '=== boot messages ==='
        sudo dmesg | grep -E \
          'stcp: directional crypto selftest passed|stcp: loopback BSD transport loaded|Undervoltage|throttl' |
          tail -100 || true

        echo
        echo '=== throttling ==='
        vcgencmd get_throttled 2>/dev/null || true
    " >"$SYSTEM_DIR/raspberry-runtime.txt" 2>&1

    if ! remote "lsmod | grep -q '^stcp '" >/dev/null 2>&1; then
        fail "STCP module is not loaded on Raspberry"
        return 1
    fi

    return 0
}

prepare_orchestrator() {
    if [[ -x "$ORCHESTRATOR" ]]; then
        return 0
    fi

    if [[ -x "$PROJECT_ROOT/orchestrate-stcp-udp-tests.sh" ]]; then
        ORCHESTRATOR="$PROJECT_ROOT/orchestrate-stcp-udp-tests.sh"
        return 0
    fi

    if [[ -x "$BENCH_DIR/orchestrate-stcp-udp-tests-fixed.sh" ]]; then
        ORCHESTRATOR="$BENCH_DIR/orchestrate-stcp-udp-tests-fixed.sh"
        return 0
    fi

    if [[ -x "$BENCH_DIR/orchestrate-stcp-udp-tests.sh" ]]; then
        ORCHESTRATOR="$BENCH_DIR/orchestrate-stcp-udp-tests.sh"
        return 0
    fi

    fail "Benchmark orchestrator not found: $ORCHESTRATOR"
    return 1
}

run_benchmarks() {
    if [[ "$SKIP_BENCHMARK" == "1" ]]; then
        log "Skipping benchmark run"
        return 0
    fi

    prepare_orchestrator || return 1

    local benchmark_result_root="$ARTIFACT_ROOT/benchmark-run"

    run_logged "benchmark-orchestrator" env \
        PROJECT_ROOT="$PROJECT_ROOT" \
        HOST="$HOST" \
        PORT="$PORT" \
        CLIENTS_LIST="$CLIENTS_LIST" \
        TEST_SCRIPT="$TEST_SCRIPT" \
        RESULT_ROOT="$benchmark_result_root" \
        ARCHIVE="$ARTIFACT_ROOT/benchmark-run.tar.gz" \
        KEEP_REMOTE_RUNNING="$KEEP_REMOTE_RUNNING" \
        CONTINUE_ON_FAILURE=1 \
        CLEAR_DMESG=1 \
        RESTART_SERVERS_EACH_RUN=1 \
        bash "$ORCHESTRATOR"

    return $?
}

collect_final_state() {
    log "Collecting final state"

    {
        date --iso-8601=seconds
        uname -a
        echo
        git status --short 2>/dev/null || true
        echo
        df -h / /boot 2>/dev/null || true
        echo
        sudo dmesg --color=never | tail -1000
    } >"$SYSTEM_DIR/devaus-after.txt" 2>&1

    remote_tty "
        set +e
        date --iso-8601=seconds
        uname -a
        echo
        lsmod | grep '^stcp' || true
        echo
        df -h / /boot 2>/dev/null || true
        echo
        vcgencmd get_throttled 2>/dev/null || true
        echo
        sudo dmesg --color=never | tail -2000
    " >"$SYSTEM_DIR/raspberry-after.txt" 2>&1 || true
}

create_summary() {
    local summary="$ARTIFACT_ROOT/SUMMARY.txt"

    {
        echo "STCP full pipeline summary"
        echo "=========================="
        echo
        echo "Run:             $RUN_NAME"
        echo "Timestamp:       $TIMESTAMP"
        echo "Git commit:      $(git rev-parse HEAD 2>/dev/null || echo unknown)"
        echo "Raspberry:       $RPI_TARGET"
        echo "Test script:     $TEST_SCRIPT"
        echo "Clients:         $CLIENTS_LIST"
        echo
        echo "Build outputs:"
        [[ -f "$X86_DIR/stcp.ko" ]] && echo "  x86: $X86_DIR/stcp.ko"
        [[ -f "$RPI_DIR/stcp.ko" ]] && echo "  RPi: $RPI_DIR/stcp.ko"
        echo
        echo "Stage statuses:"
        for status_file in "$LOG_DIR"/*.status; do
            [[ -f "$status_file" ]] || continue
            printf '  %-35s %s\n' \
                "$(basename "$status_file" .status)" \
                "$(cat "$status_file")"
        done
        echo
        echo "Artifacts:"
        echo "  Root:    $ARTIFACT_ROOT"
        echo "  Archive: $FINAL_ARCHIVE"
    } >"$summary"
}

main() {
    local pipeline_status=0

    log "STCP full pipeline started"
    log "Project root: $PROJECT_ROOT"
    log "Raspberry: $RPI_TARGET"
    log "Clients: $CLIENTS_LIST"

    check_requirements || return 1

    sudo -v || {
        fail "sudo authentication failed"
        return 1
    }

    if ! remote "echo connected" >/dev/null 2>&1; then
        fail "Cannot connect to $RPI_TARGET"
        return 1
    fi

    collect_local_system_info
    collect_remote_system_info

    if ! build_x86; then
        pipeline_status=1
    fi

    if (( pipeline_status == 0 )) && ! build_rpi; then
        pipeline_status=1
    fi

    if (( pipeline_status == 0 )) && ! verify_build_outputs; then
        pipeline_status=1
    fi

    if (( pipeline_status == 0 )) && ! deploy_rpi; then
        pipeline_status=1
    fi

    if (( pipeline_status == 0 )) && ! reboot_rpi_if_requested; then
        pipeline_status=1
    fi

    if (( pipeline_status == 0 )) && ! verify_remote_runtime; then
        pipeline_status=1
    fi

    if (( pipeline_status == 0 )) && ! run_benchmarks; then
        # Benchmark failures are recorded but do not prevent final collection.
        pipeline_status=1
    fi

    collect_final_state
    create_summary

    log "Creating final archive: $FINAL_ARCHIVE"
    tar -C "$(dirname "$ARTIFACT_ROOT")" \
        -czf "$FINAL_ARCHIVE" \
        "$(basename "$ARTIFACT_ROOT")"
    local archive_rc=$?

    if (( archive_rc != 0 )); then
        log "Failed to create final archive"
        pipeline_status=1
    fi

    log "Pipeline complete"
    log "Artifacts: $ARTIFACT_ROOT"
    log "Archive: $FINAL_ARCHIVE"

    return "$pipeline_status"
}

main "$@"
exit $?
