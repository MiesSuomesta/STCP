#!/usr/bin/env bash

set -Eeuo pipefail

IP="${IP:-192.168.1.199}"
RUSER="${RUSER:-pi}"
WAIT_TIMEOUT="${WAIT_TIMEOUT:-900}"

GIT_ROOT="$(git rev-parse --show-toplevel 2>/dev/null)" || {
    echo "[FAIL] Run this inside the STCP Git repository." >&2
    exit 1
}

SDK_ROOT="${SDK_ROOT:-$HOME/SDK/v2}"
SUMMARY_FILE="${SUMMARY_FILE:-$GIT_ROOT/full-recompile-summary.txt}"

START_EPOCH="$(date +%s)"
STARTED_AT="$(date --iso-8601=seconds)"

RPI_BUILD_STATUS="NOT_RUN"
HOST_BUILD_STATUS="NOT_RUN"
RPI_WAIT_STATUS="NOT_RUN"
BUNDLE_STATUS="NOT_RUN"

RPI_BUILD_SECS=0
HOST_BUILD_SECS=0
RPI_WAIT_SECS=0
BUNDLE_SECS=0

FINAL_EXIT=0

log() {
    printf '[INFO] %s\n' "$*"
}

warn() {
    printf '[WARN] %s\n' "$*" >&2
}

format_duration() {
    local total="${1:-0}"
    printf '%02dm %02ds' "$((total / 60))" "$((total % 60))"
}

run_timed() {
    local status_var="$1"
    local seconds_var="$2"
    local title="$3"
    shift 3

    local started rc elapsed
    started="$(date +%s)"
    log "$title"

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
        FINAL_EXIT="$rc"
    fi

    return "$rc"
}

wait_for_raspberry() {
    local deadline=$((SECONDS + WAIT_TIMEOUT))

    while ! ping -c 1 -W 1 "$IP" >/dev/null 2>&1; do
        if (( SECONDS >= deadline )); then
            echo "[FAIL] Raspberry did not answer ping within ${WAIT_TIMEOUT}s." >&2
            return 1
        fi
        log "Waiting for $IP to answer ping..."
        sleep 2
    done

    while ! ssh \
        -o BatchMode=yes \
        -o ConnectTimeout=3 \
        -o StrictHostKeyChecking=accept-new \
        "${RUSER}@${IP}" true >/dev/null 2>&1
    do
        if (( SECONDS >= deadline )); then
            echo "[FAIL] Raspberry SSH did not become ready within ${WAIT_TIMEOUT}s." >&2
            return 1
        fi
        log "Ping works; waiting for SSH on ${RUSER}@${IP}..."
        sleep 2
    done

    return 0
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
    find "$GIT_ROOT" -maxdepth 3 -type f \
        \( -name 'support-bundle*.zip' -o -name 'support-bundle.zip' \) \
        -printf '%T@ %p\n' 2>/dev/null |
        sort -nr |
        head -n 1 |
        cut -d' ' -f2-
}

cd "$GIT_ROOT"

if ! run_timed RPI_BUILD_STATUS RPI_BUILD_SECS \
    "Building and installing Raspberry kernel and modules..." \
    bash "$GIT_ROOT/STCPv2/RaspberryPI/build-rpi-package.sh"
then
    warn "Raspberry build/install failed; skipping remaining stages."
else
    if ! run_timed HOST_BUILD_STATUS HOST_BUILD_SECS \
        "Building and installing host STCP module..." \
        bash "$GIT_ROOT/STCPv2/linux-kernel/build-host-debs.sh"
    then
        warn "Host module build/install failed; bundle will still be attempted."
    fi

    if ! run_timed RPI_WAIT_STATUS RPI_WAIT_SECS \
        "Waiting for Raspberry to reboot and SSH to become ready..." \
        wait_for_raspberry
    then
        warn "Raspberry did not become ready; bundle will still be attempted."
    fi
fi

# make-support-bundle.sh runs Robot tests and creates the diagnostic bundle.
run_timed BUNDLE_STATUS BUNDLE_SECS \
    "Running Robot tests and creating support bundle..." \
    bash "$GIT_ROOT/make-support-bundle.sh" || true

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

if [[ -n "$FAIL_COUNT" && "$FAIL_COUNT" =~ ^[0-9]+$ ]]; then
    if (( FAIL_COUNT == 0 )); then
        TRAFFIC_LIGHT="ALL TESTS PASSED"
        FINAL_EXIT=0
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
Started         : $STARTED_AT
Finished        : $FINISHED_AT
Elapsed         : $(format_duration "$TOTAL_SECS")

------------------------------------------------------------
Build and deployment
------------------------------------------------------------

[RPI]   Kernel + modules        $RPI_BUILD_STATUS    $(format_duration "$RPI_BUILD_SECS")
[HOST]  STCP module             $HOST_BUILD_STATUS    $(format_duration "$HOST_BUILD_SECS")
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

STCPv2 HEAD    : $STCP_HEAD
SDK HEAD       : $SDK_HEAD

============================================================
$TRAFFIC_LIGHT
============================================================
SUMMARY
}

write_summary | tee "$SUMMARY_FILE"

exit "$FINAL_EXIT"
