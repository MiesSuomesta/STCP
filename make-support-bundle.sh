#!/usr/bin/env bash
#
# Create a flat STCPv2 source support bundle from Git-tracked files.
#
# Usage:
#   ./make-support-bundle.sh
#   ./make-support-bundle.sh --output /tmp/support-bundle.zip
#   ./make-support-bundle.sh --ref HEAD
#   ./make-support-bundle.sh --recompile
#

set -Eeuo pipefail

SDK_ROOT="${SDK_ROOT:-$HOME/SDK/v2}"

usage() {
    cat <<USAGE
Usage: $(basename "$0") [OPTIONS]

Options:
  --output FILE   Output zip path (default: ./support-bundle.zip)
  --ref REF       Git revision to archive (default: HEAD)
  --recompile     Do full recompile; reuse the Robot results it produces
  --no-robot      Do not run Robot; reuse latest results if available
  -h, --help      Show this help
USAGE
}

TS=$(date +"%d.%m.%Y_%H%M%S")
OUTPUT_BUP_DIR="${PWD}/support-bundles"
OUTPUT="${OUTPUT_BUP_DIR}/support-bundle-${TS}.zip"
REF="HEAD"
RUN_ROBOT_TESTS=1
RECOMPILE=0
RECOMPILE_RET="not-run"
TEST_STATUS="NOT_RUN"
TEST_EXIT="not-run"

while (( $# > 0 )); do
    case "$1" in
        --output)
            (( $# >= 2 )) || { echo "[FAIL] --output requires a file" >&2; exit 2; }
            OUTPUT="$2"
            shift 2
            ;;
        --ref)
            (( $# >= 2 )) || { echo "[FAIL] --ref requires a revision" >&2; exit 2; }
            REF="$2"
            shift 2
            ;;
        --recompile)
            RECOMPILE=1
            shift
            ;;
        --no-robot)
            RUN_ROBOT_TESTS=0
            shift
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "[FAIL] Unknown argument: $1" >&2
            usage >&2
            exit 2
            ;;
    esac
done

command -v git >/dev/null 2>&1 || { echo "[FAIL] git not found" >&2; exit 1; }
command -v zip >/dev/null 2>&1 || { echo "[FAIL] zip not found" >&2; exit 1; }
command -v unzip >/dev/null 2>&1 || { echo "[FAIL] unzip not found" >&2; exit 1; }
command -v sha256sum >/dev/null 2>&1 || { echo "[FAIL] sha256sum not found" >&2; exit 1; }

GIT_ROOT="$(git rev-parse --show-toplevel 2>/dev/null)" || {
    echo "[FAIL] Not inside an STCP Git repository." >&2
    exit 1
}

[[ -d "$SDK_ROOT" ]] || {
    echo "[FAIL] SDK root not found: $SDK_ROOT" >&2
    exit 1
}

OUTPUT="$(realpath -m "$OUTPUT")"
mkdir -p "$(dirname "$OUTPUT")"

TMPD="$(mktemp -d "${TMPDIR:-/tmp}/stcp-support.XXXXXX")"
cleanup() {
    rm -rf "$TMPD"
}
trap cleanup EXIT

log()  { printf '[INFO] %s\n' "$*"; }
warn() { printf '[WARN] %s\n' "$*" >&2; }
fail() { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

bundle() {
    local rel="$1"
    local out_name="$2"
    local dir="$GIT_ROOT/$rel"
    local repo_root repo_rel treeish

    [[ -d "$dir" ]] || fail "Missing source directory: $rel"

    repo_root="$(git -C "$dir" rev-parse --show-toplevel 2>/dev/null)" ||
        fail "Not a Git work tree: $rel"

    repo_rel="$(realpath --relative-to="$repo_root" "$dir")"

    git -C "$repo_root" rev-parse --verify "${REF}^{commit}" >/dev/null 2>&1 ||
        fail "Revision '$REF' not found for: $rel"

    log "Doing snapshot of $rel"
    git -C "$repo_root" add -u
    if ! git -C "$repo_root" diff --cached --quiet; then
        git -C "$repo_root" commit -m 'Snapshot commit for support package creation.' ||
            fail "Snapshot commit failed for: $rel"
    else
        log "Nothing new to commit for $rel"
    fi

    if [[ "$repo_rel" == "." ]]; then
        treeish="$REF"
    else
        treeish="${REF}:${repo_rel}"
        git -C "$repo_root" cat-file -e "$treeish" 2>/dev/null || {
            warn "No tracked tree at $rel for revision $REF; skipping."
            return 0
        }
    fi

    log "Archiving $rel -> $out_name"
    git -C "$repo_root" archive \
        --format=zip \
        --output="$TMPD/$out_name" \
        "$treeish"

    if ! unzip -Z1 "$TMPD/$out_name" | grep -q .; then
        rm -f "$TMPD/$out_name"
        warn "Archive was empty and was omitted: $out_name"
    fi
}
if (( RECOMPILE )); then
    log "Doing full recompile..."

    if SUMMARY_FILE="$TMPD/full-recompile-summary.txt" \
        bash "$GIT_ROOT/STCPv2/full-recompile.sh" |& tee "$TMPD/full-recompile.log"; then
        RECOMPILE_RET=0
        TEST_STATUS="PASS"
        TEST_EXIT=0
        log "Full recompile DONE."
    else
        RECOMPILE_RET=$?
        TEST_STATUS="FAIL"
        TEST_EXIT=$RECOMPILE_RET
        warn "Full recompile returned: $RECOMPILE_RET"
    fi
fi

bundle "STCPv2/linux-kernel" \
       "linux-kernel.zip"

bundle "STCPv2/linux-kernel/linux-module" \
       "linux-kernel-linux-module.zip"

bundle "STCPv2/RaspberryPI/raspberry-kernel-sources" \
       "raspberry-kernel-sources.zip"

bundle "STCPv2/RaspberryPI/benchmark" \
       "raspberry-benchmark.zip"

bundle "STCPv2/zephyr/nordic/echo-server" \
       "zephyr-nordic-echo-server.zip"

bundle "STCPv2/zephyr/nordic/stcp-module" \
       "zephyr-nordic-nRF9151-stcp-module.zip"

bundle "STCPv2/zephyr/nordic/stcp-application" \
       "zephyr-nordic-nRF9151-stcp-application.zip"

{
    echo "created_at=$(date --iso-8601=seconds)"
    echo "git_root=$GIT_ROOT"
    echo "requested_ref=$REF"
    echo "recompile_requested=$RECOMPILE"
    echo "recompile_exit=$RECOMPILE_RET"
    echo "outer_head=$(git -C "$GIT_ROOT" rev-parse HEAD 2>/dev/null || true)"
    echo "outer_describe=$(git -C "$GIT_ROOT" describe --always --dirty --tags 2>/dev/null || true)"
    echo
    echo "sdk_head=$(git -C "$SDK_ROOT" rev-parse HEAD 2>/dev/null || true)"
    echo "full_recompile_log=full-recompile.log"
} > "$TMPD/MANIFEST.txt"


pushd "$SDK_ROOT" >/dev/null

ROBOT_LOGS="robot-results/latest.zip"

if (( RECOMPILE )); then
    log "Using Robot results produced by full recompile..."
else
    if (( RUN_ROBOT_TESTS )); then
        log "Running Robot Framework tests..."
        if bash scripts/run-robot-tests.sh; then
            TEST_STATUS="PASS"
            TEST_EXIT=0
        else
            TEST_EXIT=$?
            TEST_STATUS="FAIL"
        fi
    else
        warn "SKIPPED: Robot tests; reusing latest results if available."
        TEST_STATUS="SKIPPED"
        TEST_EXIT="not-run"
    fi
fi

if [[ -f "$ROBOT_LOGS" ]]; then
    log "Copying Robot logs..."
    cp -av "$ROBOT_LOGS" "$TMPD/robot-test-results.zip"

    echo "robot_test_status=$TEST_STATUS" >> "$TMPD/MANIFEST.txt"
    echo "robot_test_exit=$TEST_EXIT" >> "$TMPD/MANIFEST.txt"

    # summary.txt may be at ZIP root or under the run-directory prefix.
    SUMMARY_ENTRY="$(
        unzip -Z1 "$ROBOT_LOGS" 2>/dev/null \
            | awk '/(^|\/)summary\.txt$/ { print; exit }'
    )"

    SUMMARY=""
    if [[ -n "$SUMMARY_ENTRY" ]]; then
        SUMMARY="$(unzip -p "$ROBOT_LOGS" "$SUMMARY_ENTRY" 2>/dev/null || true)"
    fi

    if [[ -n "$SUMMARY" ]]; then
        echo >> "$TMPD/MANIFEST.txt"
        echo "robot_summary:" >> "$TMPD/MANIFEST.txt"
        printf '%s\n' "$SUMMARY" | sed 's/^/  /' >> "$TMPD/MANIFEST.txt"

        PASS="$(printf '%s\n' "$SUMMARY" | sed -n 's/.*PASS=\([0-9]\+\).*/\1/p' | head -n1)"
        FAIL_COUNT="$(printf '%s\n' "$SUMMARY" | sed -n 's/.*FAIL=\([0-9]\+\).*/\1/p' | head -n1)"
        TOTAL="$(printf '%s\n' "$SUMMARY" | sed -n 's/.*TOTAL=\([0-9]\+\).*/\1/p' | head -n1)"
        REGRESSIONS="$(printf '%s\n' "$SUMMARY" | sed -n 's/.*REGRESSIONS=\([0-9]\+\).*/\1/p' | head -n1)"
        FIXED="$(printf '%s\n' "$SUMMARY" | sed -n 's/.*FIXED=\([0-9]\+\).*/\1/p' | head -n1)"
        STILL_FAILING="$(printf '%s\n' "$SUMMARY" | sed -n 's/.*STILL_FAILING=\([0-9]\+\).*/\1/p' | head -n1)"

        [[ -n "$PASS" && -n "$TOTAL" ]] && echo "robot_result=${PASS}/${TOTAL}" >> "$TMPD/MANIFEST.txt"
        [[ -n "$PASS" ]] && echo "robot_pass=$PASS" >> "$TMPD/MANIFEST.txt"
        [[ -n "$FAIL_COUNT" ]] && echo "robot_fail=$FAIL_COUNT" >> "$TMPD/MANIFEST.txt"
        [[ -n "$TOTAL" ]] && echo "robot_total=$TOTAL" >> "$TMPD/MANIFEST.txt"
        [[ -n "$REGRESSIONS" ]] && echo "robot_regressions=$REGRESSIONS" >> "$TMPD/MANIFEST.txt"
        [[ -n "$FIXED" ]] && echo "robot_fixed=$FIXED" >> "$TMPD/MANIFEST.txt"
        [[ -n "$STILL_FAILING" ]] && echo "robot_still_failing=$STILL_FAILING" >> "$TMPD/MANIFEST.txt"
    else
        warn "summary.txt was not found inside $ROBOT_LOGS"
    fi

    SHA="$(sha256sum "$ROBOT_LOGS" | awk '{print $1}')"
    echo "robot_logs_sha256=$SHA" >> "$TMPD/MANIFEST.txt"
else
    if (( RUN_ROBOT_TESTS || RECOMPILE )); then
        fail "Missing Robot log archive after requested test/recompile: $SDK_ROOT/$ROBOT_LOGS"
    fi

    warn "No Robot log archive available; continuing because --no-robot was used."
    echo "robot_test_status=SKIPPED" >> "$TMPD/MANIFEST.txt"
    echo "robot_test_exit=not-run" >> "$TMPD/MANIFEST.txt"
fi

popd >/dev/null


{
    echo
    echo "archives:"
    find "$TMPD" -maxdepth 1 -type f -name '*.zip' -printf '%f\n' | sort
} >> "$TMPD/MANIFEST.txt"

rm -f "$OUTPUT" "$OUTPUT.sha256"
(
    cd "$TMPD" || exit 1

    files=(./*.zip MANIFEST.txt)
    [[ -f full-recompile-summary.txt ]] && files+=(full-recompile-summary.txt)
    [[ -f full-recompile.log ]] && files+=(full-recompile.log)

    zip -q "$OUTPUT" "${files[@]}"
)

[[ -s "$OUTPUT" ]] || fail "Output archive was not created"
sha256sum "$OUTPUT" > "$OUTPUT.sha256"

# OUTPUT already points at its final destination. Do not copy it into a
# second "support-bundles/" directory below itself.
OUTPUT_DIR="$(dirname "$OUTPUT")"
mkdir -p "$OUTPUT_DIR"

# Keep a stable symlink beside the generated archive.
ln -sfn "$(basename "$OUTPUT")" "$OUTPUT_DIR/latest-support-package.zip"
ln -sfn "$(basename "$OUTPUT.sha256")" "$OUTPUT_DIR/latest-support-package.zip.sha256"

log "Created: $OUTPUT"
log "SHA256: $(awk '{print $1}' "$OUTPUT.sha256")"
log "Contents:"
unzip -Z1 "$OUTPUT" | sed 's/^/  /'

# Preserve the test/recompile result as the command's exit code after the
# support bundle has always been created.
if [[ "$TEST_EXIT" =~ ^[0-9]+$ ]]; then
    exit "$TEST_EXIT"
fi

pncnote -a "STCPv2 support module" "Support package done" "$(basename "$OUTPUT")"

exit 0
