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

cleanup_stcp_runtime() {
    info "Cleaning STCP users and kernel runtime..."

    pkill -TERM -x stcp-echo-server 2>/dev/null || true
    pkill -TERM -x stcp-echo-client 2>/dev/null || true
    pkill -TERM -x stcp-v2-bench-server 2>/dev/null || true
    pkill -TERM -x stcp-server 2>/dev/null || true

    sleep 1

    pkill -KILL -x stcp-echo-server 2>/dev/null || true
    pkill -KILL -x stcp-echo-client 2>/dev/null || true
    pkill -KILL -x stcp-v2-bench-server 2>/dev/null || true
    pkill -KILL -x stcp-server 2>/dev/null || true

    sleep 1

    if lsmod | grep -q '^stcp '; then
        info "Reloading STCP kernel module..."

        sudo modprobe -r stcp || {
            echo "[WARN] STCP module unload failed"

            cat /sys/module/stcp/refcnt 2>/dev/null || true
            ps -eo pid,ppid,user,comm,args | grep '[s]tcp' || true

            return 1
        }

        sudo modprobe stcp
    fi

    ssh lja@fuji "echo > /var/log/stcp/netconsole/wire.log"

    ok "STCP runtime cleaned"
}

zephyr-env()
{
    local workspace=""
    local dir="$PWD"
    local venv=""
    local req=""

    # Etsi west-workspacen juuri nykyisestä hakemistosta ylöspäin.
    while [[ "$dir" != "/" ]]; do
        if [[ -d "$dir/zephyr" && -f "$dir/.west/config" ]]; then
            workspace="$dir"
            break
        fi
        dir="$(dirname "$dir")"
    done

    # Jos ollaan workspace-puun ulkopuolella, käytä oletusta.
    if [[ -z "$workspace" ]]; then
        workspace="$HOME/zephyr-stcp"

        if [[ ! -d "$workspace/zephyr" ||
              ! -f "$workspace/.west/config" ]]; then
            echo "[FAIL] Zephyr workspace not found."
            echo "       Checked current directory tree and:"
            echo "       $workspace"
            return 1
        fi
    fi

    venv="$workspace/.venv"

    echo "[INFO] Zephyr workspace: $workspace"

    # Luo venv tarvittaessa.
    if [[ ! -d "$venv" ]]; then
        echo "[INFO] Creating Python environment: $venv"
        python3 -m venv "$venv" || return 1

        echo "[INFO] Activating Python environment..."
        source "$venv/bin/activate" || return 1

        echo "[INFO] Updating pip..."
        python -m pip install --upgrade pip || return 1

        # Ensimmäisellä luonnilla asennetaan Zephyr requirements.
        if [[ -f "$workspace/zephyr/scripts/requirements.txt" ]]; then
            echo "[INFO] Installing Zephyr requirements..."
            python -m pip install \
                -r "$workspace/zephyr/scripts/requirements.txt" || return 1
        fi

        # NCS requirements, jos ne löytyvät tästä workspacesta.
        for req in \
            "$workspace/nrf/scripts/requirements.txt" \
            "$workspace/nrf/scripts/requirements-base.txt"
        do
            if [[ -f "$req" ]]; then
                echo "[INFO] Installing: $req"
                python -m pip install -r "$req" || return 1
            fi
        done
    else
        echo "[INFO] Activating existing Python environment..."
        source "$venv/bin/activate" || return 1
    fi

    # West tarvittaessa.
    if ! python -m west --version >/dev/null 2>&1; then
        echo "[INFO] Installing west..."
        python -m pip install west || return 1
    fi

    export ZEPHYR_BASE="$workspace/zephyr"

    if [[ -d "$HOME/zephyr-sdk-0.16.8" ]]; then
        export ZEPHYR_SDK_INSTALL_DIR="$HOME/zephyr-sdk-0.16.8"
    fi

    cd "$workspace" || return 1

    echo
    echo "========================================"
    echo "[OK] Zephyr environment active"
    echo "========================================"
    echo "Workspace   : $workspace"
    echo "Python      : $(command -v python)"
    echo "West        : $(command -v west 2>/dev/null || echo unavailable)"
    echo "ZEPHYR_BASE : $ZEPHYR_BASE"
    echo "SDK         : ${ZEPHYR_SDK_INSTALL_DIR:-not set}"
    echo

    set -x
    export NCS_DIR="$workspace"
    export STCP_MODULE_DIR="$workspace/stcp/module"
    export ZEPHYR_BASE="$workspace/zephyr"
    export ZEPHYR_SDK_INSTALL_DIR="$HOME/zephyr-sdk-0.16.8"
    set +x

    python --version
    python -m west --version
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

    ps aux |grep stcp-echo-server |grep -v grep | awk '{ print "kill -9 " $2 }' |bash || true

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
    info "Cleanup before host + Raspberry Pi STCP build..."

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
    local rc=0

    info "Running Linux/Raspberry Robot regression suite..."
    cd ~/SDK/v2

    info "Running Linux/Raspberry robot tests....."

    if bash scripts/run-robot-tests.sh; then
        ok "Linux/Raspberry Robot regression PASS"
        return 0
    else
        rc=$?
        info "Linux/Raspberry Robot regression FAIL rc=$rc"
        bash scripts/stcp-postmortem.sh || true
        return "$rc"
    fi
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
            return 1
        fi
    fi
    RC=$?
    ok "Zephyr STCPv2 Robot regression suite done, rc=$RC"
    return $RC
}

main() {
    info "=================================================="
    info " STCPv2 FULL BUILD / DEPLOY / TEST RUN"
    info "=================================================="
    info "Cleanup before host + Raspberry Pi STCP build..."

    cleanup_stcp_users

    # Build/flash and run Zephyr first. This ensures that
    # /home/pomo/zephyr-stcp/stcp/application/testing/robot-v2/results/latest
    # already contains the current modem test run before the Linux/RPi Robot
    # suite and its postmortem/result-note collection are started.
    run_zephyr_build_flash

    info "Waiting 3 seconds after Zephyr flash..."
    sleep 3

    # Start the Zephyr run with a clean receiver log so a failure postmortem
    # contains only this run's netconsole traffic.
    ssh lja@fuji "echo > /var/log/stcp/netconsole/wire.log" || true

    if run_zephyr_tests; then
        ok "Zephyr STCPv2 Robot regression PASS"
    else
        zephyr_rc=$?
        info "Zephyr Robot tests FAIL rc=$zephyr_rc"
        bash ~/SDK/v2/scripts/stcp-postmortem.sh || true
        fail "Stopping full run after Zephyr Robot failure rc=$zephyr_rc"
    fi

    run_host_rpi_build_install

    # Allow USB/J-Link/console/network endpoints to settle after flash.

    # Zephyr tests may leave host-side STCP sockets/processes behind.
    # Return to a clean host/RPi baseline before running the main Robot suite.
    cleanup_stcp_users

    info "Setting up netconsole...."
    bash ~/SDK/v2/scripts/netconsole/enable-netconsole.sh

    if run_host_rpi_tests; then
        :
    else
        host_rpi_rc=$?
        fail "Stopping full run after Linux/Raspberry Robot failure rc=$host_rpi_rc"
    fi

    ok "=================================================="
    ok " FULL STCPv2 RUN PASSED"
    ok " Host/RPi build+install : PASS"
    ok " Zephyr build+flash     : PASS"
    ok " Zephyr Robot           : PASS"
    ok " Host/RPi Robot         : PASS"
    ok "=================================================="
}

main "$@"
