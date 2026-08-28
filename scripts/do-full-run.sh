#!/bin/bash
set -euo pipefail

sudo renice -n -20 $$

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
    pkill -TERM -x minicom 2>/dev/null || true

    sleep 1

    pkill -KILL -x stcp-echo-server 2>/dev/null || true
    pkill -KILL -x stcp-echo-client 2>/dev/null || true
    pkill -KILL -x stcp-v2-bench-server 2>/dev/null || true
    pkill -KILL -x stcp-server 2>/dev/null || true
    pkill -KILL -x minicom 2>/dev/null || true

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
    local stcp_repo="$HOME/STCP/STCPv2"
    local rust_core="$stcp_repo/kernel/module/rust"
    local rust_arm_target="$rust_core/target/thumbv8m.main-none-eabi"

    info "Loading Zephyr environment..."
    zephyr-env

    # Zephyr links the canonical shared Rust core directly from:
    #   kernel/module/rust/target/thumbv8m.main-none-eabi/release/libstcp_kernel_core.a
    #
    # Always remove the ARM target tree before the Zephyr clean build so a
    # stale staticlib can never survive source/overlay changes.
    [[ -f "$rust_core/Cargo.toml" ]] ||         fail "Canonical Rust core not found: $rust_core/Cargo.toml"

    if [[ -e "$rust_arm_target" ]]; then
        info "Cleaning canonical Rust ARM target: $rust_arm_target"
        rm -rf -- "$rust_arm_target"
        ok "Canonical Rust ARM target cleaned"
    else
        info "Canonical Rust ARM target already clean: $rust_arm_target"
    fi

    cd ~/zephyr-stcp/stcp/application

    info "Building Zephyr STCPv2 clean image..."
    bash scripts/build-v2-clean.sh

    # Verify that the canonical ARM staticlib was rebuilt by this build.
    local rust_staticlib="$rust_core/target/thumbv8m.main-none-eabi/release/libstcp_kernel_core.a"
    [[ -s "$rust_staticlib" ]] ||         fail "Canonical Rust ARM staticlib missing after build: $rust_staticlib"

    info "Rust ARM staticlib rebuilt:"
    ls -lh "$rust_staticlib"

    info "Flashing Zephyr STCPv2 image..."
    bash scripts/flash-v2-clean.sh

    ok "Zephyr build + flash complete"
}


collect_zephyr_server_logs() {
    local robot_dir="$1"
    local run_dir="$2"
    local artifacts_dir="$run_dir/artifacts/server"
    local found=0
    local f=""

    mkdir -p "$artifacts_dir"

    # Robot Process library writes these relative paths from the suite.
    # Copy them into the timestamped run before results/latest is published,
    # so stcp-postmortem.sh can collect the exact server logs for this run.
    for f in \
        "$robot_dir/server.stdout.log" \
        "$robot_dir/server.stderr.log"
    do
        if [[ -f "$f" ]]; then
            cp -a -- "$f" "$artifacts_dir/"
            found=1
        fi
    done

    # Also collect any future/alternate server log names without failing
    # when none exist.
    while IFS= read -r -d '' f; do
        case "$(basename "$f")" in
            server.stdout.log|server.stderr.log)
                continue
                ;;
        esac
        cp -a -- "$f" "$artifacts_dir/"
        found=1
    done < <(
        find "$robot_dir" -maxdepth 1 -type f \
            \( -name 'server*.log' -o -name 'stcp-v2-bench-server*.log' \) \
            -print0 2>/dev/null
    )

    if (( found )); then
        ok "Zephyr server logs collected: $artifacts_dir"
        ls -lh "$artifacts_dir" || true
    else
        info "No Zephyr server logs found to collect from $robot_dir"
    fi
}

run_zephyr_tests() {
    local zephyr_root="$HOME/zephyr-stcp/stcp/application"
    local robot_dir="$zephyr_root/testing/robot-v2"
    local results_dir="$robot_dir/results"
    local run_id=""
    local run_dir=""
    local latest_tmp=""
    local suite=""
    local rc=0

    info "Running Zephyr STCPv2 Robot regression suite..."

    cleanup_stcp_users

    mkdir -p "$results_dir"
    cd "$robot_dir"

    if [[ -f ./zephyr-v2.robot ]]; then
        suite="./zephyr-v2.robot"
    elif [[ -f ./robot-v2.robot ]]; then
        suite="./robot-v2.robot"
    elif [[ -f ./tests.robot ]]; then
        suite="./tests.robot"
    else
        fail "No Zephyr Robot suite found in $robot_dir"
        return 1
    fi

    # Use a collision-safe timestamped directory owned by this full run.
    run_id="$(date +%Y%m%d-%H%M%S)"
    run_dir="$results_dir/$run_id"

    # Extremely unlikely, but avoid reusing an existing directory when two
    # launches happen within the same second.
    if [[ -e "$run_dir" ]]; then
        run_id="${run_id}-$$"
        run_dir="$results_dir/$run_id"
    fi

    mkdir -p "$run_dir"

    {
        printf 'run_id=%s\n' "$run_id"
        printf 'started_at=%s\n' "$(date --iso-8601=seconds)"
        printf 'suite=%s\n' "$suite"
        printf 'cwd=%s\n' "$robot_dir"
    } >"$run_dir/run-meta.txt"

    info "Zephyr Robot run directory: $run_dir"
    info "Zephyr Robot suite        : $suite"

    # Remove stale Process-library logs before this run. Otherwise a failed
    # server start could make postmortem accidentally collect an older run.
    rm -f --         "$robot_dir/server.stdout.log"         "$robot_dir/server.stderr.log"
    find "$robot_dir" -maxdepth 1 -type f         \( -name 'server*.log' -o -name 'stcp-v2-bench-server*.log' \)         -delete 2>/dev/null || true

    # Run Robot directly so output.xml/log.html/report.html are guaranteed to
    # belong to THIS run. Do not let set -e abort before results/latest is
    # updated on a failing test.
    if robot --exitonfailure --outputdir "$run_dir" "$suite"; then
        rc=0
    else
        rc=$?
    fi

    {
        printf 'finished_at=%s\n' "$(date --iso-8601=seconds)"
        printf 'robot_rc=%s\n' "$rc"
    } >>"$run_dir/run-meta.txt"

    # Preserve bench-server stdout/stderr in THIS timestamped Robot run before
    # results/latest is published and before any postmortem collection starts.
    collect_zephyr_server_logs "$robot_dir" "$run_dir"

    # Atomically publish latest AFTER Robot has closed its result files.
    # This is done for both PASS and FAIL so postmortem always sees the run
    # that just finished.
    latest_tmp="$results_dir/.latest.$$"
    rm -f "$latest_tmp"
    ln -s "$run_id" "$latest_tmp"
    mv -Tf "$latest_tmp" "$results_dir/latest"

    # Also publish the exact selected run in a plain text file. This makes
    # postmortem diagnostics independent of symlink interpretation.
    printf '%s\n' "$run_dir" >"$results_dir/latest-run-path.txt"

    info "Zephyr results/latest -> $(readlink -f "$results_dir/latest")"
    info "Zephyr STCPv2 Robot regression suite done, rc=$rc"

    return "$rc"
}


run_zephyr_app_build_flash() {
    local app_name="$1"
    local app_root="$HOME/zephyr-stcp/stcp/$app_name"

    [[ -d "$app_root" ]] || fail "Zephyr application missing: $app_root"
    [[ -x "$app_root/scripts/build-v2-clean.sh" || -f "$app_root/scripts/build-v2-clean.sh" ]] || \
        fail "Build script missing: $app_root/scripts/build-v2-clean.sh"
    [[ -x "$app_root/scripts/flash-v2-clean.sh" || -f "$app_root/scripts/flash-v2-clean.sh" ]] || \
        fail "Flash script missing: $app_root/scripts/flash-v2-clean.sh"

    info "Building Zephyr application: $app_name"
    cd "$app_root"
    bash scripts/build-v2-clean.sh

    info "Flashing Zephyr application: $app_name"
    bash scripts/flash-v2-clean.sh

    info "Waiting 3 seconds after $app_name flash..."
    sleep 3

    ok "Zephyr application build + flash complete: $app_name"
}

run_zephyr_app_tests() {
    local app_name="$1"
    local app_root="$HOME/zephyr-stcp/stcp/$app_name"
    local runner="$app_root/scripts/run-application-robot.sh"
    local rc=0

    [[ -f "$runner" ]] || fail "Application Robot runner missing: $runner"

    info "Running $app_name application regression..."

    # App suites own their gateway/backend processes. Kill stale STCP users
    # and Minicom before handing the serial port and AF_STCP sockets to them.
    cleanup_stcp_users

    cd "$app_root"

    if bash "$runner"; then
        ok "$app_name application regression PASS"
        return 0
    else
        rc=$?
        info "$app_name application regression FAIL rc=$rc"
        return "$rc"
    fi
}

run_coap_regression() {
    local rc=0

    run_zephyr_app_build_flash "app-coap"

    if run_zephyr_app_tests "app-coap"; then
        return 0
    else
        rc=$?
        return "$rc"
    fi
}

run_mqtt_regression() {
    local rc=0

    run_zephyr_app_build_flash "app-mqtt"

    if run_zephyr_app_tests "app-mqtt"; then
        return 0
    else
        rc=$?
        return "$rc"
    fi
}

restore_zephyr_golden_image() {
    info "Restoring normal Zephyr STCPv2 test application..."
    cleanup_stcp_users
    run_zephyr_build_flash
    info "Waiting 3 seconds after golden Zephyr restore..."
    sleep 3
    ok "Normal Zephyr STCPv2 test application restored"
}

main() {
    info "=================================================="
    info " STCPv2 FULL BUILD / DEPLOY / TEST RUN"
    info "=================================================="
    info "Cleanup before host + Raspberry Pi STCP build..."

    cleanup_stcp_users


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

    # Build/flash and run Zephyr first. This ensures that
    # /home/pomo/zephyr-stcp/stcp/application/testing/robot-v2/results/latest
    # already contains the current modem test run before the Linux/RPi Robot
    # suite and its postmortem/result-note collection are started.
    cleanup_stcp_users

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
        info "Collecting postmortem from finalized Zephyr results/latest..."
        bash ~/SDK/v2/scripts/stcp-postmortem.sh || true
        fail "Stopping full run after Zephyr Robot failure rc=$zephyr_rc"
    fi

    # Golden transport regression has passed. Application regressions are
    # deliberately run afterwards so a CoAP/MQTT application failure cannot
    # hide a transport regression.
    cleanup_stcp_users

    if run_coap_regression; then
        ok "Zephyr CoAP application regression PASS"
    else
        coap_rc=$?
        info "Zephyr CoAP application regression FAIL rc=$coap_rc"
        # Best effort: leave the board in the normal Robot-v2 firmware even
        # after an application-suite failure.
        restore_zephyr_golden_image || true
        fail "Stopping full run after CoAP application failure rc=$coap_rc"
    fi

    cleanup_stcp_users

    if run_mqtt_regression; then
        ok "Zephyr MQTT application regression PASS"
    else
        mqtt_rc=$?
        info "Zephyr MQTT application regression FAIL rc=$mqtt_rc"
        restore_zephyr_golden_image || true
        fail "Stopping full run after MQTT application failure rc=$mqtt_rc"
    fi

    # app-mqtt is the last firmware flashed above. Always put the normal
    # command-driven test application back on the board, otherwise a later
    # manual robot-v2 run sees "stcp: command not found".
    restore_zephyr_golden_image

    ok "=================================================="
    ok " FULL STCPv2 RUN PASSED"
    ok " Host/RPi build+install : PASS"
    ok " Zephyr build+flash     : PASS"
    ok " Zephyr Robot           : PASS"
    ok " Host/RPi Robot         : PASS"
    ok " Zephyr CoAP app        : PASS"
    ok " Zephyr MQTT app        : PASS"
    ok " Golden Zephyr restore  : PASS"
    ok "=================================================="
}

main "$@"
