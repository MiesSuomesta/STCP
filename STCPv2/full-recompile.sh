#!/usr/bin/env bash

set -Eeuo pipefail

IP="${IP:-raspi}"
RUSER="${RUSER:-pi}"
WAIT_TIMEOUT="${WAIT_TIMEOUT:-900}"

GIT_ROOT="$(git rev-parse --show-toplevel 2>/dev/null)" || {
    echo "[FAIL] Run this inside the STCP Git repository." >&2
    exit 1
}

STCP_ROOT="${STCP_ROOT:-$GIT_ROOT/STCPv2}"
LINUX_MODULE_ROOT="${LINUX_MODULE_ROOT:-$STCP_ROOT/linux-kernel/linux-module}"
SDK_ROOT="${SDK_ROOT:-$HOME/SDK/v2}"
SUMMARY_FILE="${SUMMARY_FILE:-$GIT_ROOT/full-recompile-summary.txt}"

# Override these from the environment if your tree uses different wrappers.
RPI_BUILD_SCRIPT="${RPI_BUILD_SCRIPT:-$STCP_ROOT/RaspberryPI/build-rpi-package.sh}"
HOST_BUILD_SCRIPT="${HOST_BUILD_SCRIPT:-$STCP_ROOT/linux-kernel/build-host-debs.sh}"
SUPPORT_BUNDLE_SCRIPT="${SUPPORT_BUNDLE_SCRIPT:-$GIT_ROOT/make-support-bundle.sh}"

START_EPOCH="$(date +%s)"
STARTED_AT="$(date --iso-8601=seconds)"

SOURCE_CHECK_STATUS="NOT_RUN"
RPI_BUILD_STATUS="NOT_RUN"
HOST_BUILD_STATUS="NOT_RUN"
SDK_HOST_STATUS="NOT_RUN"
SDK_RPI_STATUS="NOT_RUN"
RPI_WAIT_STATUS="NOT_RUN"
BUNDLE_STATUS="NOT_RUN"

SOURCE_CHECK_SECS=0
RPI_BUILD_SECS=0
HOST_BUILD_SECS=0
SDK_HOST_SECS=0
SDK_RPI_SECS=0
RPI_WAIT_SECS=0
BUNDLE_SECS=0

FINAL_EXIT=0

TS=$(date +"%d%m%Y-%H%M%S")
VERSION_PREFIX=${VERSION_PREFIX:-stcp}
LOCALVERSION_COMMON=${LOCALVERSION:--${TS}-${VERSION_PREFIX}}

LOCALVERSION_HOST="${LOCALVERSION_COMMON}-host"
LOCALVERSION_RPI="${LOCALVERSION_COMMON}-rpi"

echo "[INFO] VERSION_PREFIX=$VERSION_PREFIX"
echo "[INFO] LOCALVERSION_COMMON=$LOCALVERSION_COMMON"
echo "[INFO] LOCALVERSION_HOST=$LOCALVERSION_HOST"
echo "[INFO] LOCALVERSION_RPI=$LOCALVERSION_RPI"

log() {
    printf '[INFO] %s\n' "$*"
}

warn() {
    printf '[WARN] %s\n' "$*" >&2
}

fail() {
    printf '[FAIL] %s\n' "$*" >&2
}

format_duration() {
    local total="${1:-0}"
    printf '%02dm %02ds' "$((total / 60))" "$((total % 60))"
}

remember_failure() {
    local rc="$1"
    if (( rc != 0 && FINAL_EXIT == 0 )); then
        FINAL_EXIT="$rc"
    fi
}

run_timed() {
    local status_var="$1"
    local seconds_var="$2"
    local title="$3"
    shift 3

    local started rc elapsed
    started="$(date +%s)"
    log "$title"
    echo "[RUN TIMED] $@"
    set +e
    "$@"
    rc=$?
    set -e

    elapsed=$(( $(date +%s) - started ))
    printf -v "$seconds_var" '%d' "$elapsed"

    if (( rc == 0 )); then
        printf -v "$status_var" '%s' "OK"
    else
        printf -v "$status_var" '%s' "FAIL($rc)"
        remember_failure "$rc"
    fi

    return "$rc"
}

check_unified_source_tree() {
    if [[ ! -d "$LINUX_MODULE_ROOT/src" ]]; then
        fail "Canonical Linux module source is missing: $LINUX_MODULE_ROOT/src"
        return 1
    fi

    if [[ ! -d "$LINUX_MODULE_ROOT/rust/src" ]]; then
        fail "Canonical Rust core is missing: $LINUX_MODULE_ROOT/rust/src"
        return 1
    fi

    local old_rpi_src="$STCP_ROOT/RaspberryPI/raspberry-kernel-module/src"
    local old_rpi_rust="$STCP_ROOT/RaspberryPI/raspberry-kernel-module/rust/src"

    if [[ -d "$old_rpi_src" || -d "$old_rpi_rust" ]]; then
        fail "Duplicate Raspberry kernel-module source tree still exists."
        [[ -d "$old_rpi_src" ]] && fail "  $old_rpi_src"
        [[ -d "$old_rpi_rust" ]] && fail "  $old_rpi_rust"
        fail "Raspberry must build from: $LINUX_MODULE_ROOT"
        return 1
    fi

    log "Unified Linux module source: $LINUX_MODULE_ROOT"
    return 0
}

wait_for_raspberry() {
    local deadline=$((SECONDS + WAIT_TIMEOUT))
    log "Starting to wait for Rapsberry ($IP) to respond..."
    until ping -c 1 -W 1 "$IP" >/dev/null 2>&1; do
        if (( SECONDS >= deadline )); then
            fail "Raspberry did not answer ping within ${WAIT_TIMEOUT}s."
            return 1
        fi
        log "Waiting for $IP to answer ping..."
        sleep 2
    done

    log "Starting to wait for Rapsberry ($IP) SSH service to come up..."
    until ssh \
        -o BatchMode=yes \
        -o ConnectTimeout=3 \
        -o StrictHostKeyChecking=accept-new \
        "${RUSER}@${IP}" true >/dev/null 2>&1
    do
        if (( SECONDS >= deadline )); then
            fail "Raspberry SSH did not become ready within ${WAIT_TIMEOUT}s."
            return 1
        fi
        log "Ping works; waiting for SSH on ${RUSER}@${IP}..."
        sleep 2
    done

    return 0
}

build_sdk_host() {
    if [[ ! -d "$SDK_ROOT" ]]; then
        fail "SDK root does not exist: $SDK_ROOT"
        return 1
    fi

    cd "$SDK_ROOT"

    if [[ -x "$SDK_ROOT/scripts/build-host.sh" ]]; then
        bash "$SDK_ROOT/scripts/build-host.sh"
        return
    fi

    if [[ -x "$SDK_ROOT/build-host.sh" ]]; then
        bash "$SDK_ROOT/build-host.sh"
        return
    fi

    if [[ -f "$SDK_ROOT/Cargo.toml" ]]; then
        cargo clean
        cargo build --workspace --all-targets
        return
    fi

    fail "No host SDK build entry point found under $SDK_ROOT"
    return 1
}

build_sdk_rpi() {
    if [[ ! -d "$SDK_ROOT" ]]; then
        fail "SDK root does not exist: $SDK_ROOT"
        return 1
    fi

    cd "$SDK_ROOT"

    if [[ -x "$SDK_ROOT/scripts/build-rpi.sh" ]]; then
        bash "$SDK_ROOT/scripts/build-rpi.sh"
        return
    fi

    if [[ -x "$SDK_ROOT/build-rpi.sh" ]]; then
        bash "$SDK_ROOT/build-rpi.sh"
        return
    fi

    if [[ -f "$SDK_ROOT/Cargo.toml" ]]; then
        cargo build --workspace --all-targets --target aarch64-unknown-linux-gnu
        return
    fi

    fail "No Raspberry SDK build entry point found under $SDK_ROOT"
    return 1
}

find_robot_summary() {
    local candidate

    for candidate in \
        "$SDK_ROOT/robot-results/latest/rfa-comparison.txt" \
        "$SDK_ROOT/robot-results/latest/summary.txt" \
        "$SDK_ROOT/robot-results/rfa-comparison.txt"
    do
        if [[ -f "$candidate" ]]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done

    return 1
}

find_latest_bundle() {
    find "$GIT_ROOT" -maxdepth 4 -type f \
        \( -name 'support-bundle*.zip' -o -name 'support-bundle.zip' \) \
        -printf '%T@ %p\n' 2>/dev/null |
        sort -nr |
        head -n 1 |
        cut -d' ' -f2-
}

cd "$GIT_ROOT"

# ---------------------------------------------------------------------------
# 1. Verify that x86_64 and Raspberry now share one canonical module source.
# ---------------------------------------------------------------------------
if ! run_timed SOURCE_CHECK_STATUS SOURCE_CHECK_SECS \
    "Checking unified Linux kernel-module source tree..." \
    check_unified_source_tree
then
    warn "Source tree is not unified. Stopping before builds."
else
    # -----------------------------------------------------------------------
    # 2. Raspberry kernel/package. The wrapper must build STCP from
    #    $LINUX_MODULE_ROOT rather than an old Raspberry source copy.
    # -----------------------------------------------------------------------


    log "Compiling kernel for host,      version: $LOCALVERSION_HOST"
    log "Compiling kernel for Raspberry, version: $LOCALVERSION_RPI"


    if [[ ! -f "$RPI_BUILD_SCRIPT" ]]; then
        RPI_BUILD_STATUS="FAIL(missing)"
        remember_failure 1
        warn "Raspberry build script not found: $RPI_BUILD_SCRIPT"
    else
        run_timed RPI_BUILD_STATUS RPI_BUILD_SECS \
            "Building Raspberry kernel/package + unified STCP module..." \
            env STCP_LINUX_MODULE_ROOT="$LINUX_MODULE_ROOT" \
                LINUX_MODULE_ROOT="$LINUX_MODULE_ROOT" \
                LOCALVERSION="$LOCALVERSION_RPI" \
                    bash "$RPI_BUILD_SCRIPT" || true
    fi

    # -----------------------------------------------------------------------
    # 3. Host kernel module from the very same source tree.
    # -----------------------------------------------------------------------
    if [[ ! -f "$HOST_BUILD_SCRIPT" ]]; then
        HOST_BUILD_STATUS="FAIL(missing)"
        remember_failure 1
        warn "Host build script not found: $HOST_BUILD_SCRIPT"
    else
        run_timed HOST_BUILD_STATUS HOST_BUILD_SECS \
            "Building and installing host unified STCP module..." \
             env STCP_LINUX_MODULE_ROOT="$LINUX_MODULE_ROOT" \
                 LINUX_MODULE_ROOT="$LINUX_MODULE_ROOT" \
                 LOCALVERSION="$LOCALVERSION_HOST" \
                    bash "$HOST_BUILD_SCRIPT" || true
    fi

    # -----------------------------------------------------------------------
    # 4. SDK: clean/rebuild both Linux host and Raspberry targets.
    # -----------------------------------------------------------------------
    run_timed SDK_HOST_STATUS SDK_HOST_SECS \
        "Clean-building SDK for host..." \
        build_sdk_host || true

    run_timed SDK_RPI_STATUS SDK_RPI_SECS \
        "Building SDK for Raspberry (aarch64)..." \
        build_sdk_rpi || true

    # -----------------------------------------------------------------------
    # 5. Raspberry may reboot during package/kernel installation.
    # -----------------------------------------------------------------------
    run_timed RPI_WAIT_STATUS RPI_WAIT_SECS \
        "Waiting for Raspberry to reboot and SSH to become ready..." \
        wait_for_raspberry || true
fi

# ---------------------------------------------------------------------------
# 6. Robot matrix + diagnostic support bundle.
# ---------------------------------------------------------------------------
(
	cd "${STCP_ROOT}"
	if [[ ! -f "$SUPPORT_BUNDLE_SCRIPT" ]]; then
	    BUNDLE_STATUS="FAIL(missing)"
	    remember_failure 1
	    warn "Support-bundle script not found: $SUPPORT_BUNDLE_SCRIPT"
	else
	    run_timed BUNDLE_STATUS BUNDLE_SECS \
	        "Running Robot tests and creating support bundle..." \
	        bash "$SUPPORT_BUNDLE_SCRIPT" || true
	fi
)

FINISH_EPOCH="$(date +%s)"
FINISHED_AT="$(date --iso-8601=seconds)"
TOTAL_SECS=$((FINISH_EPOCH - START_EPOCH))

ROBOT_SUMMARY_FILE="$(find_robot_summary 2>/dev/null || true)"
ROBOT_CURRENT="Unavailable"
ROBOT_CHANGES="Unavailable"
PASS_COUNT=""
FAIL_COUNT=""
TOTAL_COUNT=""
REGRESSIONS=""

if [[ -n "$ROBOT_SUMMARY_FILE" ]]; then
    ROBOT_CURRENT="$(grep -m1 -E 'Current:.*PASS=.*FAIL=.*TOTAL=' "$ROBOT_SUMMARY_FILE" || true)"
    ROBOT_CHANGES="$(grep -m1 -E 'Changes:.*REGRESSIONS=' "$ROBOT_SUMMARY_FILE" || true)"

    PASS_COUNT="$(sed -n 's/.*PASS=\([0-9][0-9]*\).*/\1/p' <<<"$ROBOT_CURRENT")"
    FAIL_COUNT="$(sed -n 's/.*FAIL=\([0-9][0-9]*\).*/\1/p' <<<"$ROBOT_CURRENT")"
    TOTAL_COUNT="$(sed -n 's/.*TOTAL=\([0-9][0-9]*\).*/\1/p' <<<"$ROBOT_CURRENT")"
    REGRESSIONS="$(sed -n 's/.*REGRESSIONS=\([0-9][0-9]*\).*/\1/p' <<<"$ROBOT_CHANGES")"

    [[ -n "$ROBOT_CURRENT" ]] || ROBOT_CURRENT="Unavailable"
    [[ -n "$ROBOT_CHANGES" ]] || ROBOT_CHANGES="Unavailable"
fi

LATEST_BUNDLE="$(find_latest_bundle || true)"
BUNDLE_SHA=""
if [[ -n "$LATEST_BUNDLE" && -f "$LATEST_BUNDLE" ]]; then
    BUNDLE_SHA="$(sha256sum "$LATEST_BUNDLE" | awk '{print $1}')"
fi

STCP_HEAD="$(git -C "$GIT_ROOT" rev-parse --short HEAD 2>/dev/null || echo unknown)"
SDK_HEAD="$(git -C "$SDK_ROOT" rev-parse --short HEAD 2>/dev/null || echo unknown)"

# Preserve build failures. Robot can turn a previously-successful run into a
# failure, but it must never erase an earlier compile/deploy failure.
if [[ -n "$FAIL_COUNT" && "$FAIL_COUNT" =~ ^[0-9]+$ ]]; then
    if (( FAIL_COUNT == 0 )); then
        if (( FINAL_EXIT == 0 )); then
            TRAFFIC_LIGHT="ALL BUILDS AND TESTS PASSED"
        else
            TRAFFIC_LIGHT="TESTS PASSED, BUT A BUILD/DEPLOY STAGE FAILED"
        fi
    elif [[ "$REGRESSIONS" =~ ^[0-9]+$ ]] && (( REGRESSIONS == 0 )); then
        TRAFFIC_LIGHT="NO REGRESSIONS; ${FAIL_COUNT} TEST(S) STILL FAILING"
        (( FINAL_EXIT == 0 )) && FINAL_EXIT=1
    else
        TRAFFIC_LIGHT="REGRESSIONS OR TEST FAILURES DETECTED"
        (( FINAL_EXIT == 0 )) && FINAL_EXIT=1
    fi
else
    TRAFFIC_LIGHT="ROBOT SUMMARY UNAVAILABLE"
    (( FINAL_EXIT == 0 )) && FINAL_EXIT=1
fi

write_summary() {
    cat <<SUMMARY
============================================================
               STCP FULL REBUILD SUMMARY
============================================================

Repository      : $GIT_ROOT
STCP root       : $STCP_ROOT
Linux module    : $LINUX_MODULE_ROOT
SDK root        : $SDK_ROOT
Started         : $STARTED_AT
Finished        : $FINISHED_AT
Elapsed         : $(format_duration "$TOTAL_SECS")

------------------------------------------------------------
Source layout
------------------------------------------------------------

[CHECK] Unified source          $SOURCE_CHECK_STATUS    $(format_duration "$SOURCE_CHECK_SECS")

------------------------------------------------------------
Build and deployment
------------------------------------------------------------

[RPI]   Kernel + STCP module    $RPI_BUILD_STATUS    $(format_duration "$RPI_BUILD_SECS")
[HOST]  STCP module             $HOST_BUILD_STATUS    $(format_duration "$HOST_BUILD_SECS")
[SDK]   Host                    $SDK_HOST_STATUS    $(format_duration "$SDK_HOST_SECS")
[SDK]   Raspberry               $SDK_RPI_STATUS    $(format_duration "$SDK_RPI_SECS")
[RPI]   Reboot + SSH            $RPI_WAIT_STATUS    $(format_duration "$RPI_WAIT_SECS")
[BUNDLE] Tests + package        $BUNDLE_STATUS    $(format_duration "$BUNDLE_SECS")

------------------------------------------------------------
Robot Framework
------------------------------------------------------------

$ROBOT_CURRENT
$ROBOT_CHANGES

------------------------------------------------------------
Artifacts
------------------------------------------------------------

Support bundle : ${LATEST_BUNDLE:-not found}
SHA256         : ${BUNDLE_SHA:-not available}
Summary file   : $SUMMARY_FILE

------------------------------------------------------------
Git
------------------------------------------------------------

STCP HEAD      : $STCP_HEAD
SDK HEAD       : $SDK_HEAD

============================================================
$TRAFFIC_LIGHT
============================================================
SUMMARY
}

write_summary | tee "$SUMMARY_FILE"

exit "$FINAL_EXIT"
