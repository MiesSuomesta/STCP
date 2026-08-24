#!/bin/bash
set -euo pipefail

ts() {
    date +"[%d.%m.%Y %H:%M:%S]"
}

info() {
    echo "$(ts) [INFO] $*"
}

ok() {
    echo "$(ts) [OK] $*"
}

fail() {
    echo "$(ts) [FAIL] $*" >&2
    exit 1
}


cleanup_stcp_users() {
    local -a names=(
        stcp-echo-server
        stcp-echo-client
        stcp-v2-bench-server
        stcp-server
        stcp_large_test
    )
    local name
    local refcnt=""

    info "Cleaning processes that may hold AF_STCP sockets..."

    for name in "${names[@]}"; do
        if pgrep -x "$name" >/dev/null 2>&1; then
            info "Stopping STCP user process: $name"
            pkill -TERM -x "$name" 2>/dev/null || true
        fi
    done

    # Give normal close()/release() paths a chance to run.
    sleep 1

    for name in "${names[@]}"; do
        if pgrep -x "$name" >/dev/null 2>&1; then
            info "Killing stubborn STCP user process: $name"
            pkill -KILL -x "$name" 2>/dev/null || true
        fi
    done

    sleep 1

    if [[ -r /sys/module/stcp/refcnt ]]; then
        refcnt="$(cat /sys/module/stcp/refcnt)"
        if [[ "$refcnt" != "0" ]]; then
            echo "$(ts) [WARN] STCP module refcnt is still $refcnt after cleanup." >&2
            echo "$(ts) [WARN] Some process/socket may still hold the module." >&2

            # Best-effort process hints. Custom AF_STCP sockets are not
            # necessarily identifiable by generic fuser/lsof tooling.
            ps -eo pid,ppid,user,comm,args \
                | grep -E '[s]tcp|[e]cho-(server|client)|[b]ench-server' \
                || true
        else
            ok "STCP module refcnt is 0"
        fi
    else
        info "STCP module is not currently loaded; no refcnt to verify"
    fi
}

wait_raspi_down() {
    info "Waiting Raspberry Pi to go down..."
    for _ in $(seq 1 60); do
        if ! ping -c 1 -W 1 raspi >/dev/null 2>&1; then
            ok "Raspberry Pi is down"
            return 0
        fi
        sleep 1
    done
    fail "Raspberry Pi did not go down within 60 seconds"
}

wait_raspi_ssh() {
    info "Waiting Raspberry Pi SSH..."
    for _ in $(seq 1 180); do
        if ssh \
            -o BatchMode=yes \
            -o ConnectTimeout=2 \
            -o ConnectionAttempts=1 \
            pi@raspi true >/dev/null 2>&1
        then
            ok "Raspberry Pi SSH is ready"
            return 0
        fi
        echo "$(ts) Raspberry Pi not ready"
        sleep 1
    done
    fail "Raspberry Pi SSH did not become ready within 180 seconds"
}

run_host_rpi_build_install() {
    cleanup_stcp_users

    info "Building host + Raspberry Pi STCP..."
    cd ~/STCP/STCPv2

    bash scripts/build-all.sh host rpi
    bash scripts/install-all.sh host
    bash scripts/install-all.sh rpi

    info "Rebooting Raspberry Pi..."
    ssh pi@raspi 'sudo reboot' || true

    wait_raspi_down
    wait_raspi_ssh
}

run_host_rpi_tests() {
    info "Running Linux/Raspberry Robot regression suite..."
    cd ~/SDK/v2

    bash scripts/run-robot-tests.sh

    ok "Linux/Raspberry Robot regression suite PASS"
}

run_zephyr_build_flash() {
    info "Loading Zephyr environment..."
    zephyr-env

    cd ~/zephyr-stcp/stcp/application

    info "Building Zephyr STCPv2 clean image..."
    bash scripts/build-v2-clean.sh

    info "Flashing Zephyr STCPv2 image..."
    bash scripts/flash-v2-clean.sh

    ok "Zephyr build + flash complete"
}

run_zephyr_tests() {
    local zephyr_root="$HOME/zephyr-stcp/stcp/application"
    local robot_dir="$zephyr_root/testing/robot-v2"

    info "Running Zephyr STCPv2 Robot regression suite..."

    cleanup_stcp_users
    cd "$robot_dir"

    # Prefer a project-provided runner if present.
    if [[ -x ./run.sh ]]; then
        ./run.sh
    elif [[ -x ./run-robot.sh ]]; then
        ./run-robot.sh
    elif [[ -x ./run-tests.sh ]]; then
        ./run-tests.sh
    elif [[ -x ./run-robot-tests.sh ]]; then
        ./run-robot-tests.sh
    else
        # Known STCPv2 robot-v2 layout fallback.
        if [[ -f ./zephyr-v2.robot ]]; then
            robot zephyr-v2.robot
        elif [[ -f ./robot-v2.robot ]]; then
            robot robot-v2.robot
        elif [[ -f ./tests.robot ]]; then
            robot tests.robot
        else
            fail "No Zephyr Robot runner/test suite found in $robot_dir"
        fi
    fi

    ok "Zephyr STCPv2 Robot regression suite PASS"
}

main() {
    info "=================================================="
    info " STCPv2 FULL BUILD / DEPLOY / TEST RUN"
    info "=================================================="

    run_host_rpi_build_install

    # Validate Linux + Raspberry baseline before touching Zephyr.
    run_host_rpi_tests

    run_zephyr_build_flash

    # Allow USB/J-Link/console/network endpoints to settle after flash.
    info "Waiting 3 seconds after Zephyr flash..."
    sleep 3

    run_zephyr_tests

    ok "=================================================="
    ok " FULL STCPv2 RUN PASSED"
    ok " Host/RPi build+install : PASS"
    ok " Host/RPi Robot         : PASS"
    ok " Zephyr build+flash     : PASS"
    ok " Zephyr Robot           : PASS"
    ok "=================================================="
}

main "$@"
