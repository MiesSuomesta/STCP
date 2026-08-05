#!/usr/bin/env bash
#
# Create a flat STCPv2 source support bundle from Git-tracked files.
#
# Usage:
#   ./make-support-bundle.sh
#   ./make-support-bundle.sh --output /tmp/support-bundle.zip
#   ./make-support-bundle.sh --ref HEAD
#

set -uo pipefail

SDK_ROOT="$HOME/SDK/v2"

usage() {
    cat <<USAGE
Usage: $(basename "$0") [OPTIONS]

Options:
  --output FILE   Output zip path (default: ./support-bundle.zip)
  --ref REF       Git revision to archive (default: HEAD)
  -h, --help      Show this help
USAGE
}

OUTPUT="${PWD}/support-bundle.zip"
REF="HEAD"

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

# Resolve the repository root from wherever the script is launched.
GIT_ROOT="$(git rev-parse --show-toplevel 2>/dev/null)" || {
    echo "[FAIL] Not inside an STCP Git repository." >&2
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

# Archive one directory, regardless of whether it is part of the outer repo
# or itself a nested Git repository.
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

    if [[ "$repo_rel" == "." ]]; then
        treeish="$REF"
    else
        treeish="${REF}:${repo_rel}"
        git -C "$repo_root" cat-file -e "$treeish" 2>/dev/null || {
            warn "No tracked tree at $rel for revision $REF; skipping."
            return 0
        }
    fi

    log "Doing snapshot of $rel"
    git -C "$repo_root" add -u
    git -C "$repo_root" commit -m 'Snapshot commit for support package creation.'

    log "Archiving $rel -> $out_name"
    git -C "$repo_root" archive \
        --format=zip \
        --output="$TMPD/$out_name" \
        "$treeish"

    # Reject empty or structurally useless archives.
    if ! unzip -Z1 "$TMPD/$out_name" | grep -q .; then
        rm -f "$TMPD/$out_name"
        warn "Archive was empty and was omitted: $out_name"
        return 0
    fi
}

bundle "STCPv2/linux-kernel/linux-module" \
       "linux-kernel-module.zip"

bundle "STCPv2/RaspberryPI/raspberry-kernel-module" \
       "raspberry-kernel-module.zip"

bundle "STCPv2/RaspberryPI/benchmark" \
       "raspberry-benchmark.zip"

bundle "STCPv2/zephyr/nordic/echo-server" \
       "zephyr-nordic-echo-server.zip"

bundle "STCPv2/zephyr/nordic/stcp-module" \
       "zephyr-nordic-nRF9151-stcp-module.zip"

bundle "STCPv2/zephyr/nordic/stcp-application" \
       "zephyr-nordic-nRF9151-stcp-application.zip"

(
    ROBOT_LOGS="robot-results/latest.zip"
    cd "$SDK_ROOT" && bash scripts/run-robot-tests.sh && (
		cp -av $ROBOT_LOGS "$TMPD/robot-test-results.zip"
	)
)

# Add lightweight provenance metadata.
{
    echo "created_at=$(date --iso-8601=seconds)"
    echo "git_root=$GIT_ROOT"
    echo "requested_ref=$REF"
    echo "outer_head=$(git -C "$GIT_ROOT" rev-parse HEAD 2>/dev/null || true)"
    echo "outer_describe=$(git -C "$GIT_ROOT" describe --always --dirty --tags 2>/dev/null || true)"
    echo
    echo "sdk_head=$(git -C "$SDK_ROOT" rev-parse HEAD 2>/dev/null || true)"
    echo
    echo "archives:"
    find "$TMPD" -maxdepth 1 -type f -name '*.zip' -printf '%f\n' | sort
} > "$TMPD/MANIFEST.txt"

rm -f "$OUTPUT" "$OUTPUT.sha256"
(
    cd "$TMPD"
    zip -q "$OUTPUT" ./*.zip MANIFEST.txt
)

[[ -s "$OUTPUT" ]] || fail "Output archive was not created"
sha256sum "$OUTPUT" > "$OUTPUT.sha256"

log "Created: $OUTPUT"
log "SHA256: $(awk '{print $1}' "$OUTPUT.sha256")"
log "Contents:"
unzip -Z1 "$OUTPUT" | sed 's/^/  /'
