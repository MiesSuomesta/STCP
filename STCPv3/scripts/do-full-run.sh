#!/bin/bash
set -euo pipefail

#sudo renice -n -20 $$

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
teardown() {
    local rc=$?
    trap - EXIT INT TERM HUP
    set +e

    info "=================================================="
    info " FULL RUN TEARDOWN rc=$rc"
    info "=================================================="

    # devaus / local host
    cleanup_stcp_users

    # Raspberry Pi
    ssh -o ConnectTimeout=3 pi@raspi '
        sudo pkill -TERM -f "stcp|echo-server|echo-client|bench-server" 2>/dev/null || true
        sleep 1
        sudo pkill -KILL -f "stcp|echo-server|echo-client|bench-server" 2>/dev/null || true
    ' || true

    # Netconsole receiver host:
    # remove leaked receivers, but do NOT kill unrelated tcpdump instances.
    ssh -o ConnectTimeout=3 lja@fuji '
        pkill -TERM -f "bash /home/lja/enable-tcpdump.sh" 2>/dev/null || true
        sudo pkill -TERM -f "tcpdump -lni any -s0 -A udp dst port 6666" 2>/dev/null || true
        sleep 1
        pkill -KILL -f "bash /home/lja/enable-tcpdump.sh" 2>/dev/null || true
        sudo pkill -KILL -f "tcpdump -lni any -s0 -A udp dst port 6666" 2>/dev/null || true
    ' || true

    # Local leftovers from Robot / gateways / serial users
    pkill -TERM -f 'mqtt_proxy|coap|stcp-v2-mqtt|stcp-v2.*gateway' 2>/dev/null || true
    pkill -TERM -x minicom 2>/dev/null || true
    sleep 1
    pkill -KILL -f 'mqtt_proxy|coap|stcp-v2-mqtt|stcp-v2.*gateway' 2>/dev/null || true
    pkill -KILL -x minicom 2>/dev/null || true

    ok "FULL RUN TEARDOWN COMPLETE"
    exit "$rc"
}

trap teardown EXIT INT TERM HUP

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
        stcp-libp2p
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
    cd ~/STCP/STCPv3

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
    cd ~/SDK/version-to-use/

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


report_compression_stats() {
    local sdk_root="$HOME/SDK/version-to-use/"
    local result_root="$sdk_root/robot-results"
    local latest=""
    local report=""

    info "Collecting STCPv3 compression statistics..."

    if [[ -L "$result_root/latest" || -d "$result_root/latest" ]]; then
        latest="$(readlink -f "$result_root/latest" 2>/dev/null || true)"
    fi

    if [[ -z "$latest" || ! -d "$latest" ]]; then
        info "Compression statistics unavailable: no Robot latest run under $result_root"
        return 0
    fi

    if ! report="$(
        python3 - "$latest" <<'PY'
import re
import sys
from pathlib import Path

run_dir = Path(sys.argv[1])

pat = re.compile(
    r"\[STCP-COMPRESSION-STATS\]\s+"
    r"tx_attempts=(\d+)\s+"
    r"tx_compressed_frames=(\d+)\s+"
    r"tx_fallback_frames=(\d+)\s+"
    r"tx_input_bytes=(\d+)\s+"
    r"tx_wire_bytes=(\d+)\s+"
    r"tx_errors=(\d+)\s+"
    r"rx_compressed_frames=(\d+)\s+"
    r"rx_wire_bytes=(\d+)\s+"
    r"rx_output_bytes=(\d+)\s+"
    r"rx_errors=(\d+)"
)
keys = (
    "tx_attempts", "tx_compressed_frames", "tx_fallback_frames",
    "tx_input_bytes", "tx_wire_bytes", "tx_errors",
    "rx_compressed_frames", "rx_wire_bytes", "rx_output_bytes", "rx_errors",
)

def zero():
    return {k: 0 for k in keys}

groups = {"compressible": zero(), "incompressible": zero()}
files = {"compressible": 0, "incompressible": 0}

logs = sorted(run_dir.rglob("*-client.log")) + sorted(run_dir.rglob("*-server.log"))

for path in logs:
    try:
        s = path.read_text(encoding="utf-8", errors="replace")
    except OSError:
        continue
    ms = list(pat.finditer(s))
    if not ms:
        continue

    # New random tests use 193xx ports / random marker in artifact names.
    name = path.name.lower()
    mode = "incompressible" if ("random" in name or re.search(r"-193(?:2[1-4]|3[1-4])-", name)) else "compressible"

    vals = [int(x) for x in ms[-1].groups()]
    for k, v in zip(keys, vals):
        groups[mode][k] += v
    files[mode] += 1

if not sum(files.values()):
    print("NO_STATS=1")
    raise SystemExit(0)

def emit(prefix, d, nfiles):
    saved = max(0, d["tx_input_bytes"] - d["tx_wire_bytes"])
    reduction = saved * 100.0 / d["tx_input_bytes"] if d["tx_input_bytes"] else 0.0
    print(f"{prefix}_FILES={nfiles}")
    for k in keys:
        print(f"{prefix}_{k.upper()}={d[k]}")
    print(f"{prefix}_SAVED_BYTES={saved}")
    print(f"{prefix}_REDUCTION={reduction:.2f}")

emit("COMP", groups["compressible"], files["compressible"])
emit("RAND", groups["incompressible"], files["incompressible"])

total = zero()
for k in keys:
    total[k] = groups["compressible"][k] + groups["incompressible"][k]
emit("TOTAL", total, sum(files.values()))
PY
    )"; then
        info "Compression statistics parser failed; continuing without report"
        return 0
    fi

    if grep -q '^NO_STATS=1$' <<<"$report"; then
        info "No compression statistics markers found in: $latest"
        return 0
    fi

    eval "$report"

    echo
    ok "=================================================="
    ok " STCPv3 AUTO COMPRESSION FUNCTIONAL TEST"
    ok "=================================================="

    printf '%s\n' \
        " COMPRESSIBLE DATA" \
        " Endpoint logs            : ${COMP_FILES:-0}" \
        " Attempts                 : ${COMP_TX_ATTEMPTS:-0}" \
        " Compressed frames        : ${COMP_TX_COMPRESSED_FRAMES:-0}" \
        " Fallback frames          : ${COMP_TX_FALLBACK_FRAMES:-0}" \
        " Original payload         : ${COMP_TX_INPUT_BYTES:-0} B" \
        " Wire payload             : ${COMP_TX_WIRE_BYTES:-0} B" \
        " Payload bytes saved      : ${COMP_SAVED_BYTES:-0} B" \
        " Payload reduction        : ${COMP_REDUCTION:-0.00} %" \
        "" \
        " INCOMPRESSIBLE / RANDOM DATA (WORST CASE)" \
        " Endpoint logs            : ${RAND_FILES:-0}" \
        " Attempts                 : ${RAND_TX_ATTEMPTS:-0}" \
        " Compressed frames        : ${RAND_TX_COMPRESSED_FRAMES:-0}" \
        " Fallback frames          : ${RAND_TX_FALLBACK_FRAMES:-0}" \
        " Original payload         : ${RAND_TX_INPUT_BYTES:-0} B" \
        " Wire payload             : ${RAND_TX_WIRE_BYTES:-0} B" \
        "" \
        " INTEGRITY / ERRORS" \
        " RX compressed frames     : ${TOTAL_RX_COMPRESSED_FRAMES:-0}" \
        " RX restored payload      : ${TOTAL_RX_OUTPUT_BYTES:-0} B" \
        " Compression errors       : ${TOTAL_TX_ERRORS:-0}" \
        " Decompression errors     : ${TOTAL_RX_ERRORS:-0}"

    # Functional invariants. These make the report itself useful as a gate.
    local functional_pass=1
    (( ${COMP_TX_ATTEMPTS:-0} > 0 )) || functional_pass=0
    (( ${COMP_TX_COMPRESSED_FRAMES:-0} > 0 )) || functional_pass=0
    (( ${COMP_TX_FALLBACK_FRAMES:-0} == 0 )) || functional_pass=0
    (( ${RAND_TX_ATTEMPTS:-0} > 0 )) || functional_pass=0
    (( ${RAND_TX_COMPRESSED_FRAMES:-0} == 0 )) || functional_pass=0
    (( ${RAND_TX_FALLBACK_FRAMES:-0} > 0 )) || functional_pass=0
    (( ${TOTAL_TX_ERRORS:-0} == 0 )) || functional_pass=0
    (( ${TOTAL_RX_ERRORS:-0} == 0 )) || functional_pass=0

    if (( functional_pass )); then
        ok " AUTO COMPRESSION RESULT : PASS"
        ok " Compressible path       : PASS (COMPRESS)"
        ok " Worst-case random path  : PASS (FALLBACK)"
    else
        fail " AUTO COMPRESSION RESULT : FAIL"
        return 1
    fi

    ok "=================================================="
    echo
}

run_zephyr_build_flash() {
    local stcp_repo="$HOME/STCP/STCPv3"
    local rust_core="$stcp_repo/kernel/module/rust"
    local rust_target_triple="thumbv8m.main-none-eabi"
    local rust_arm_target="$rust_core/target/$rust_target_triple"
    local rust_staticlib="$rust_arm_target/release/libstcp_kernel_core.a"

    info "Loading Zephyr environment..."
    zephyr-env

    # Zephyr links the canonical shared Rust core directly from:
    #   kernel/module/rust/target/thumbv8m.main-none-eabi/release/libstcp_kernel_core.a
    #
    # Build it explicitly here. Do not rely on the Zephyr application build
    # to create this artifact as a side effect.
    [[ -f "$rust_core/Cargo.toml" ]] || \
        fail "Canonical Rust core not found: $rust_core/Cargo.toml"

    command -v cargo >/dev/null 2>&1 || \
        fail "cargo not found after Zephyr environment activation"

    if [[ -e "$rust_arm_target" ]]; then
        info "Cleaning canonical Rust ARM target: $rust_arm_target"
        rm -rf -- "$rust_arm_target"
        ok "Canonical Rust ARM target cleaned"
    else
        info "Canonical Rust ARM target already clean: $rust_arm_target"
    fi

    info "Building canonical Rust ARM staticlib for $rust_target_triple..."
    (
        cd "$rust_core"
        cargo build --release --target "$rust_target_triple"
    ) || fail "Canonical Rust ARM build failed for target $rust_target_triple"

    [[ -s "$rust_staticlib" ]] || \
        fail "Canonical Rust ARM staticlib missing after cargo build: $rust_staticlib"

    ok "Canonical Rust ARM staticlib rebuilt"
    ls -lh "$rust_staticlib"

    cd "$HOME/zephyr-stcp/stcp/application"

    info "Building Zephyr STCPv3 clean image..."
    bash scripts/build-v2-clean.sh

    # Verify that the canonical staticlib still exists after the Zephyr build.
    [[ -s "$rust_staticlib" ]] || \
        fail "Canonical Rust ARM staticlib disappeared after Zephyr build: $rust_staticlib"

    info "Flashing Zephyr STCPv3 image..."
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
    local robot_dir="$zephyr_root/testing/robot-v3"
    local results_dir="$robot_dir/results"
    local run_id=""
    local run_dir=""
    local latest_tmp=""
    local suite=""
    local rc=0

    info "Running Zephyr STCPv3 Robot regression suite..."

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
    info "Zephyr STCPv3 Robot regression suite done, rc=$rc"

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


run_p2p_regression() {
    local p2p_root="$HOME/zephyr-stcp/stcp/p2p-application"
    local p2p_runner="$p2p_root/testing/run-three-node-p2p.sh"
    local serial_dev="${STCP_ZEPHYR_SERIAL:-/dev/ttyACM0}"
    local rc=0

    [[ -d "$p2p_root" ]] || fail "P2P application missing: $p2p_root"
    [[ -f "$p2p_root/scripts/build.sh" ]] || fail "P2P build script missing: $p2p_root/scripts/build.sh"
    [[ -f "$p2p_root/scripts/flash.sh" ]] || fail "P2P flash script missing: $p2p_root/scripts/flash.sh"
    [[ -f "$p2p_runner" ]] || fail "P2P three-node runner missing: $p2p_runner"
    [[ -e "$serial_dev" ]] || fail "Zephyr serial device missing: $serial_dev"

    info "Running standalone Zephyr P2P three-node regression..."

    cleanup_stcp_users
    zephyr-env

    cd "$p2p_root"

    info "Building standalone P2P application..."
    bash scripts/build.sh

    info "Flashing standalone P2P application..."
    bash scripts/flash.sh

    info "Waiting 3 seconds after P2P flash..."
    sleep 3

    cleanup_stcp_users

    info "Running P2P three-node matrix..."
    info "Runner builds Linux + Raspberry Pi rust-libp2p peers and deploys the RPi binary."

    if STCP_ZEPHYR_SERIAL="$serial_dev" bash "$p2p_runner"; then
        rc=0
        ok "Zephyr standalone P2P three-node regression PASS"
    else
        rc=$?
        info "Zephyr standalone P2P three-node regression FAIL rc=$rc"
    fi

    return "$rc"
}

restore_zephyr_golden_image() {
    info "Restoring normal Zephyr STCPv3 test application..."
    cleanup_stcp_users
    run_zephyr_build_flash
    info "Waiting 3 seconds after golden Zephyr restore..."
    sleep 3
    ok "Normal Zephyr STCPv3 test application restored"
}

publish_stcp_fi_results() {
    local sdk_root="$HOME/SDK/version-to-use/"
    local publisher="$sdk_root/tools/site-generator/publish-tested-result.sh"
    local run="$sdk_root/robot-results/latest"

    [[ -f "$publisher" ]] || fail "stcp.fi publisher missing: $publisher"
    [[ -e "$run" || -L "$run" ]] || fail "STCPv3 result run missing: $run"

    info "Publishing successful STCPv3 full-run results to stcp.fi..."

    # Publication is deliberately opt-in. This explicit flag exists only on
    # the all-tests-passed path at the end of do-full-run.
    bash "$publisher" "$run" --publish-stcp-fi

    ok "STCPv3 full-run results published to stcp.fi"
}

main() {
    info "=================================================="
    info " STCPv3 FULL BUILD / DEPLOY / TEST RUN"
    info "=================================================="
    info "Cleanup before host + Raspberry Pi STCP build..."

    cleanup_stcp_users


    run_host_rpi_build_install

    # Allow USB/J-Link/console/network endpoints to settle after flash.

    # Zephyr tests may leave host-side STCP sockets/processes behind.
    # Return to a clean host/RPi baseline before running the main Robot suite.
    cleanup_stcp_users

    info "Setting up netconsole...."
    bash ~/SDK/version-to-use/scripts/netconsole/enable-netconsole.sh

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
        ok "Zephyr STCPv3 Robot regression PASS"
    else
        zephyr_rc=$?
        info "Zephyr Robot tests FAIL rc=$zephyr_rc"
        info "Collecting postmortem from finalized Zephyr results/latest..."
        bash ~/SDK/version-to-use/scripts/stcp-postmortem.sh || true
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
        # Best effort: leave the board in the normal Robot-v3 firmware even
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

    cleanup_stcp_users

    if run_p2p_regression; then
        ok "Zephyr standalone P2P application regression PASS"
    else
        p2p_rc=$?
        info "Zephyr standalone P2P application regression FAIL rc=$p2p_rc"
        restore_zephyr_golden_image || true
        fail "Stopping full run after P2P application failure rc=$p2p_rc"
    fi

    # Aggregate the compression matrix statistics collected by the
    # Linux/Raspberry Pi Robot suite.
    report_compression_stats

    # p2p-application is the last firmware flashed above. Always put the
    # normal command-driven test application back on the board.
    restore_zephyr_golden_image

    # Every required phase has now passed and the golden image was restored.
    # Only this success path is allowed to opt in to stcp.fi publication.
    publish_stcp_fi_results

    ok "=================================================="
    ok " FULL STCPv3 RUN PASSED"
    ok " Host/RPi build+install : PASS"
    ok " Zephyr build+flash     : PASS"
    ok " Zephyr Robot           : PASS"
    ok " Host/RPi Robot         : PASS"
    ok " Zephyr CoAP app        : PASS"
    ok " Zephyr MQTT app        : PASS"
    ok " Zephyr P2P 3-node      : PASS"
    ok " Golden Zephyr restore  : PASS"
    ok " stcp.fi publication    : PASS"
    ok "=================================================="
}

main "$@"
