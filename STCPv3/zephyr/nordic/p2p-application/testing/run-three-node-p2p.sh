#!/usr/bin/env bash
set -euo pipefail

# STCPv2 three-node P2P regression
#
# Current promoted matrix:
#   Linux -> RPi     full rust-libp2p: STCP + multistream + Noise XX + Yamux + ping
#   RPi   -> Linux   full rust-libp2p: STCP + multistream + Noise XX + Yamux + ping
#   Modem -> Linux   STCP + multistream + Noise XX + libp2p identity
#   Modem -> RPi     STCP + multistream + Noise XX + libp2p identity
#
# Zephyr responder/Yamux is not yet promoted, so Linux/RPi -> modem is deliberately
# not reported as a PASS. That direction becomes valid when the native Zephyr
# listener + Yamux backend exists.

HOST_SDK_ROOT="${STCP_P2P_HOST_SDK_ROOT:-$HOME/SDK/v2}"
HOST_BIN="${STCP_P2P_HOST_BIN:-$HOST_SDK_ROOT/target/release/stcp-libp2p}"
RPI_HOST="${STCP_P2P_RPI_HOST:-raspi}"
RPI_USER="${STCP_P2P_RPI_USER:-pi}"
RPI_TARGET="${STCP_P2P_RPI_TARGET:-aarch64-unknown-linux-gnu}"
RPI_CROSS_BIN="${STCP_P2P_RPI_CROSS_BIN:-$HOST_SDK_ROOT/target/$RPI_TARGET/release/stcp-libp2p}"
RPI_BIN="${STCP_P2P_RPI_BIN:-/tmp/stcp-libp2p-p2p3node}"
SERIAL_DEV="${STCP_ZEPHYR_SERIAL:-/dev/ttyACM0}"
LINUX_IP="${STCP_P2P_LINUX_IP:-192.168.1.20}"
RPI_IP="${STCP_P2P_RPI_IP:-}"
BASE_PORT="${STCP_P2P_BASE_PORT:-19010}"
TIMEOUT="${STCP_P2P_TIMEOUT:-30}"
RPI_LINKER="${STCP_P2P_RPI_LINKER:-aarch64-linux-gnu-gcc}"
RESULT_ROOT="${STCP_P2P_RESULT_ROOT:-$(pwd)/testing/results}"
RUN_ID="$(date +%Y%m%d-%H%M%S)"
RUN_DIR="$RESULT_ROOT/p2p-3node-$RUN_ID"
SSH=(ssh -o BatchMode=yes -o ConnectTimeout=5 "$RPI_USER@$RPI_HOST")
SCP=(scp -q -o BatchMode=yes -o ConnectTimeout=5)
LOCAL_PIDS=()
REMOTE_PIDFILES=()

mkdir -p "$RUN_DIR"

log() { printf '[P2P-3NODE] %s\n' "$*"; }
pass() { printf '[P2P-3NODE] PASS: %s\n' "$*"; }
fail() { printf '[P2P-3NODE] FAIL: %s\n' "$*" >&2; exit 1; }

remote_stop_pidfile() {
    local pidfile=$1
    "${SSH[@]}" "
        if [ -s '$pidfile' ]; then
            pid=\$(cat '$pidfile' 2>/dev/null || true)
            case \"\$pid\" in
                ''|*[!0-9]*) ;;
                *)
                    kill -TERM \"\$pid\" 2>/dev/null || true
                    i=0
                    while kill -0 \"\$pid\" 2>/dev/null && [ \"\$i\" -lt 20 ]; do
                        sleep 0.1
                        i=\$((i + 1))
                    done
                    kill -KILL \"\$pid\" 2>/dev/null || true
                    ;;
            esac
            rm -f '$pidfile'
        fi
    " >/dev/null 2>&1 || true
}

remote_cleanup_stale() {
    # Cleanup from older runner versions which used pkill -x stcp-libp2p.
    # The deployed executable is named stcp-libp2p-p2p3node, so those runs
    # could leave listeners alive and keep 19011/19012 occupied.
    "${SSH[@]}" "
        pids=\$(pgrep -f '^${RPI_BIN}( |$)' 2>/dev/null || true)
        for pid in \$pids; do kill -TERM \"\$pid\" 2>/dev/null || true; done
        sleep 0.2
        for pid in \$pids; do kill -KILL \"\$pid\" 2>/dev/null || true; done
    " >/dev/null 2>&1 || true
}

remote_start() {
    local pidfile=$1 logfile=$2; shift 2
    remote_stop_pidfile "$pidfile"
    REMOTE_PIDFILES+=("$pidfile")
    local quoted=() arg
    for arg in "$@"; do quoted+=("$(printf '%q' "$arg")"); done
    "${SSH[@]}" "rm -f '$pidfile' '$logfile'; nohup env STCP_P2P_TRACE=1 ${quoted[*]} >'$logfile' 2>&1 </dev/null & echo \$! >'$pidfile'"
}

cleanup() {
    local rc=$?
    set +e
    for pid in "${LOCAL_PIDS[@]:-}"; do
        kill -TERM "$pid" 2>/dev/null || true
    done
    sleep 0.2
    for pid in "${LOCAL_PIDS[@]:-}"; do
        kill -KILL "$pid" 2>/dev/null || true
        wait "$pid" 2>/dev/null || true
    done
    for pidfile in "${REMOTE_PIDFILES[@]:-}"; do
        remote_stop_pidfile "$pidfile"
    done
    remote_cleanup_stale
    "${SSH[@]}" "rm -f '$RPI_BIN'" >/dev/null 2>&1 || true
    log "results: $RUN_DIR"
    exit "$rc"
}
trap cleanup EXIT INT TERM HUP

wait_log() {
    local file=$1 pattern=$2 seconds=${3:-$TIMEOUT}
    local end=$((SECONDS + seconds))
    while (( SECONDS < end )); do
        if grep -Eq "$pattern" "$file" 2>/dev/null; then
            return 0
        fi
        sleep 0.1
    done
    return 1
}

wait_remote_log() {
    local file=$1 pattern=$2 seconds=${3:-$TIMEOUT}
    local end=$((SECONDS + seconds))
    while (( SECONDS < end )); do
        if "${SSH[@]}" "grep -Eq $(printf '%q' "$pattern") $(printf '%q' "$file") 2>/dev/null"; then
            return 0
        fi
        sleep 0.2
    done
    return 1
}

stop_local_pid() {
    local pid=$1
    kill -TERM "$pid" 2>/dev/null || true
    for _ in $(seq 1 20); do
        kill -0 "$pid" 2>/dev/null || break
        sleep 0.1
    done
    kill -KILL "$pid" 2>/dev/null || true
    wait "$pid" 2>/dev/null || true
}

resolve_rpi_ip() {
    if [[ -n "$RPI_IP" ]]; then
        return 0
    fi
    RPI_IP="$(getent ahostsv4 "$RPI_HOST" 2>/dev/null | awk 'NR==1{print $1}')"
    if [[ -z "$RPI_IP" ]]; then
        RPI_IP="$("${SSH[@]}" "hostname -I | awk '{print \\$1}'" 2>/dev/null || true)"
    fi
    [[ -n "$RPI_IP" ]] || fail "cannot resolve Raspberry Pi IPv4 (set STCP_P2P_RPI_IP)"
}

ensure_rpi_stcp_module() {
    log "ensuring STCP kernel module on Raspberry Pi"

    if ! "${SSH[@]}" '
        set -e

        if ! lsmod | grep -q "^stcp "; then
            echo "[RPi] STCP module not loaded; loading with modprobe"
            sudo modprobe stcp
        fi

        lsmod | grep "^stcp "
        test -r /sys/module/stcp/refcnt
        echo "[RPi] STCP module refcnt=$(cat /sys/module/stcp/refcnt)"
    '; then
        fail "STCP kernel module unavailable on Raspberry Pi"
    fi

    pass "RPi STCP kernel module ready"
}

build_peers() {
    local linker_env="CARGO_TARGET_AARCH64_UNKNOWN_LINUX_GNU_LINKER"

    command -v cargo >/dev/null 2>&1 || fail "cargo not found on Linux build host"
    command -v rustc >/dev/null 2>&1 || fail "rustc not found on Linux build host"

    log "building Linux stcp-libp2p"
    (cd "$HOST_SDK_ROOT" && \
        cargo build --release -p stcp-libp2p \
            --no-default-features --features platform-linux)
    [[ -x "$HOST_BIN" ]] || fail "Linux binary missing: $HOST_BIN"

    log "cross-building Raspberry Pi stcp-libp2p on Linux target=$RPI_TARGET"

    # The Raspberry Pi remains a pure test target: no cargo/rustc is required
    # on the Pi.  The Linux build host produces the aarch64 binary and deploys
    # it over SSH.
    if command -v rustup >/dev/null 2>&1; then
        if ! rustup target list --installed | grep -qx "$RPI_TARGET"; then
            log "installing Rust target on Linux build host: $RPI_TARGET"
            rustup target add "$RPI_TARGET" || \
                fail "cannot install Rust target $RPI_TARGET"
        fi
    else
        # Non-rustup installations can still have the target preinstalled.
        # Let cargo provide the authoritative error if it is absent.
        log "rustup not found; assuming Rust target $RPI_TARGET is already installed"
    fi

    if command -v "$RPI_LINKER" >/dev/null 2>&1; then
        log "RPi cross linker: $(command -v "$RPI_LINKER")"
        (
            cd "$HOST_SDK_ROOT"
            env "$linker_env=$RPI_LINKER" \
                cargo build --release \
                    --target "$RPI_TARGET" \
                    -p stcp-libp2p \
                    --no-default-features \
                    --features platform-rpi
        )
    else
        log "cross linker '$RPI_LINKER' not found in PATH"
        log "trying Cargo project linker configuration for $RPI_TARGET"
        (
            cd "$HOST_SDK_ROOT"
            cargo build --release \
                --target "$RPI_TARGET" \
                -p stcp-libp2p \
                --no-default-features \
                --features platform-rpi
        ) || fail "RPi cross-build failed; install/provide $RPI_LINKER or set STCP_P2P_RPI_LINKER"
    fi

    [[ -x "$RPI_CROSS_BIN" ]] || \
        fail "RPi cross-built binary missing: $RPI_CROSS_BIN"

    # Sanity-check the produced ELF when the host has file(1).
    if command -v file >/dev/null 2>&1; then
        log "RPi binary: $(file -b "$RPI_CROSS_BIN")"
        file -b "$RPI_CROSS_BIN" | grep -Eqi 'ARM aarch64|ARM64' || \
            fail "cross-built RPi binary is not AArch64: $RPI_CROSS_BIN"
    fi

    log "cleaning stale Raspberry Pi P2P processes"
    remote_cleanup_stale

    log "deploying Raspberry Pi binary to $RPI_USER@$RPI_HOST:$RPI_BIN"
    "${SSH[@]}" "rm -f '$RPI_BIN'"
    "${SCP[@]}" "$RPI_CROSS_BIN" "$RPI_USER@$RPI_HOST:$RPI_BIN"
    "${SSH[@]}" "chmod 0755 '$RPI_BIN' && test -x '$RPI_BIN'" || \
        fail "RPi binary deploy failed: $RPI_BIN"

    pass "RPi stcp-libp2p cross-build + deploy"
}

run_linux_to_rpi() {
    local port=$((BASE_PORT + 1))
    local rlog="/tmp/stcp-p2p-rpi-listener-$RUN_ID.log"
    local rpid="/tmp/stcp-p2p-rpi-listener-$RUN_ID.pid"
    local clog="$RUN_DIR/linux-to-rpi-client.log"
    local slog="$RUN_DIR/linux-to-rpi-server.log"

    log "Linux -> RPi full libp2p on $RPI_IP:$port"
    remote_start "$rpid" "$rlog" "$RPI_BIN" --listen "/ip4/0.0.0.0/tcp/$port"
    wait_remote_log "$rlog" 'libp2p listener ready:' 10 || {
        "${SSH[@]}" "cat '$rlog'" >"$slog" 2>&1 || true
        fail "RPi listener did not become ready"
    }

    STCP_P2P_TRACE=1 "$HOST_BIN" \
        --listen /ip4/0.0.0.0/tcp/$((port + 100)) \
        --dial /ip4/$RPI_IP/tcp/$port >"$clog" 2>&1 &
    local pid=$!
    LOCAL_PIDS+=("$pid")

    if ! wait_log "$clog" 'P2P CONNECTED peer=' "$TIMEOUT" || \
       ! wait_log "$clog" 'P2P PING peer=.*result=Ok' "$TIMEOUT"; then
        "${SSH[@]}" "cat '$rlog'" >"$slog" 2>&1 || true
        tail -n 120 "$clog" >&2 || true
        tail -n 120 "$slog" >&2 || true
        fail "Linux -> RPi did not reach CONNECTED + ping"
    fi

    "${SSH[@]}" "cat '$rlog'" >"$slog" 2>&1 || true
    stop_local_pid "$pid"
    remote_stop_pidfile "$rpid"
    pass "Linux -> RPi: STCP + Noise XX + Yamux + ping"
}

run_rpi_to_linux() {
    local port=$((BASE_PORT + 2))
    local slog="$RUN_DIR/rpi-to-linux-server.log"
    local rlog="/tmp/stcp-p2p-rpi-dialer-$RUN_ID.log"
    local rpid="/tmp/stcp-p2p-rpi-dialer-$RUN_ID.pid"
    local clog="$RUN_DIR/rpi-to-linux-client.log"

    log "RPi -> Linux full libp2p on $LINUX_IP:$port"
    STCP_P2P_TRACE=1 "$HOST_BIN" --listen /ip4/0.0.0.0/tcp/$port >"$slog" 2>&1 &
    local spid=$!
    LOCAL_PIDS+=("$spid")
    wait_log "$slog" 'libp2p listener ready:' 10 || fail "Linux listener did not become ready"

    remote_start "$rpid" "$rlog" "$RPI_BIN" \
        --listen "/ip4/0.0.0.0/tcp/$((port + 100))" \
        --dial "/ip4/$LINUX_IP/tcp/$port"

    if ! wait_remote_log "$rlog" 'P2P CONNECTED peer=' "$TIMEOUT" || \
       ! wait_remote_log "$rlog" 'P2P PING peer=.*result=Ok' "$TIMEOUT"; then
        "${SSH[@]}" "cat '$rlog'" >"$clog" 2>&1 || true
        tail -n 120 "$clog" >&2 || true
        tail -n 120 "$slog" >&2 || true
        fail "RPi -> Linux did not reach CONNECTED + ping"
    fi

    "${SSH[@]}" "cat '$rlog'" >"$clog" 2>&1 || true
    remote_stop_pidfile "$rpid"
    stop_local_pid "$spid"
    pass "RPi -> Linux: STCP + Noise XX + Yamux + ping"
}

serial_noise_probe() {
    local target_ip=$1 target_port=$2 label=$3 output=$4
    python3 - "$SERIAL_DEV" "$target_ip" "$target_port" "$label" >"$output" 2>&1 <<'PY'
import serial
import sys
import time

dev, host, port, label = sys.argv[1:]
ser = serial.Serial(dev, 115200, timeout=0.1)

def command(cmd, needle="p2p>", timeout=8.0):
    ser.write((cmd + "\r\n").encode())
    ser.flush()
    end = time.monotonic() + timeout
    data = ""
    while time.monotonic() < end:
        b = ser.read(4096)
        if b:
            data += b.decode(errors="replace")
            if needle in data:
                print(data, end="")
                return data
        else:
            time.sleep(0.02)
    raise RuntimeError(f"{label}: timeout after {cmd!r}; received:\n{data[-12000:]}")

try:
    ser.reset_input_buffer()

    # Synchronize robustly with the Zephyr shell.  Depending on when the serial
    # device is opened, reset_input_buffer() may discard the last visible prompt.
    # Keep nudging the shell with CRLF until a fresh p2p> prompt is observed.
    end = time.monotonic() + 10.0
    next_enter = 0.0
    data = ""
    while time.monotonic() < end:
        now = time.monotonic()
        if now >= next_enter:
            ser.write(b"\r\n")
            ser.flush()
            next_enter = now + 0.5

        b = ser.read(4096)
        if b:
            data += b.decode(errors="replace")
            if "p2p>" in data:
                break
        else:
            time.sleep(0.02)

    if "p2p>" not in data:
        raise RuntimeError(
            f"{label}: shell prompt not found; received:\n{data[-12000:]}"
        )

    command(f"stcp p2p host {host}")
    command(f"stcp p2p port {port}")
    show = command("stcp p2p show", timeout=30.0)
    if "Noise core  : XX+identity linked selftest=PASS (0)" not in show:
        raise RuntimeError(f"{label}: Noise selftest not PASS")

    noise = command("stcp p2p noise", timeout=30.0)
    required = (
        "multistream : PASS",
        "/noise      : PASS",
        "Noise XX + libp2p identity: PASS",
    )
    missing = [x for x in required if x not in noise]
    if missing:
        raise RuntimeError(f"{label}: missing " + ", ".join(missing))
finally:
    ser.close()
PY
}

run_modem_to_linux() {
    local port=$((BASE_PORT + 3))
    local slog="$RUN_DIR/modem-to-linux-server.log"
    local mlog="$RUN_DIR/modem-to-linux-serial.log"

    log "Modem -> Linux Noise XX interop on $LINUX_IP:$port"
    STCP_P2P_TRACE=1 "$HOST_BIN" --listen /ip4/0.0.0.0/tcp/$port >"$slog" 2>&1 &
    local pid=$!
    LOCAL_PIDS+=("$pid")
    wait_log "$slog" 'libp2p listener ready:' 10 || fail "Linux P2P listener did not become ready for modem"

    if ! serial_noise_probe "$LINUX_IP" "$port" "modem->linux" "$mlog"; then
        tail -n 160 "$mlog" >&2 || true
        tail -n 160 "$slog" >&2 || true
        fail "Modem -> Linux Noise XX interop"
    fi
    stop_local_pid "$pid"
    pass "Modem -> Linux: STCP + multistream + Noise XX identity"
}

run_modem_to_rpi() {
    local port=$((BASE_PORT + 4))
    local rlog="/tmp/stcp-p2p-rpi-modem-listener-$RUN_ID.log"
    local rpid="/tmp/stcp-p2p-rpi-modem-listener-$RUN_ID.pid"
    local slog="$RUN_DIR/modem-to-rpi-server.log"
    local mlog="$RUN_DIR/modem-to-rpi-serial.log"

    log "Modem -> RPi Noise XX interop on $RPI_IP:$port"
    remote_start "$rpid" "$rlog" "$RPI_BIN" --listen "/ip4/0.0.0.0/tcp/$port"
    wait_remote_log "$rlog" 'libp2p listener ready:' 10 || {
        "${SSH[@]}" "cat '$rlog'" >"$slog" 2>&1 || true
        fail "RPi P2P listener did not become ready for modem"
    }

    if ! serial_noise_probe "$RPI_IP" "$port" "modem->rpi" "$mlog"; then
        "${SSH[@]}" "cat '$rlog'" >"$slog" 2>&1 || true
        tail -n 160 "$mlog" >&2 || true
        tail -n 160 "$slog" >&2 || true
        fail "Modem -> RPi Noise XX interop"
    fi
    "${SSH[@]}" "cat '$rlog'" >"$slog" 2>&1 || true
    remote_stop_pidfile "$rpid"
    pass "Modem -> RPi: STCP + multistream + Noise XX identity"
}

main() {
    [[ -e "$SERIAL_DEV" ]] || fail "Zephyr serial device missing: $SERIAL_DEV"
    "${SSH[@]}" true || fail "Raspberry Pi SSH unavailable: $RPI_USER@$RPI_HOST"
    resolve_rpi_ip
    ensure_rpi_stcp_module

    cat >"$RUN_DIR/matrix.txt" <<EOF_MATRIX
Linux IP : $LINUX_IP
RPi IP   : $RPI_IP
Serial   : $SERIAL_DEV
Base port: $BASE_PORT
RPi target: $RPI_TARGET
RPi binary: $RPI_BIN

PROMOTED:
  Linux -> RPi   full libp2p/Noise/Yamux/ping
  RPi   -> Linux full libp2p/Noise/Yamux/ping
  Modem -> Linux multistream/Noise XX/identity
  Modem -> RPi   multistream/Noise XX/identity

NOT PROMOTED YET:
  Linux -> Modem native libp2p/Yamux
  RPi   -> Modem native libp2p/Yamux
EOF_MATRIX

    log "Linux=$LINUX_IP RPi=$RPI_IP serial=$SERIAL_DEV"
    build_peers
    run_linux_to_rpi
    run_rpi_to_linux
    run_modem_to_linux
    run_modem_to_rpi

    pass "THREE-NODE P2P MATRIX"
    log "Linux <-> RPi full P2P: PASS"
    log "Modem -> Linux Noise interop: PASS"
    log "Modem -> RPi Noise interop: PASS"
    log "Linux/RPi -> modem remains gated on Zephyr native listener + Yamux"
}

main "$@"
