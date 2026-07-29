#!/usr/bin/env bash
set -Eeuo pipefail

# Back up the current benchmark site and replace it atomically.
#
# Usage:
#   deploy-site.sh GENERATED_DIR LIVE_DIR
#
# Environment:
#   BACKUP_ROOT=/var/backups/stcp-benchmark
#   KEEP_BACKUPS=10

GENERATED_DIR="${1:?Usage: $0 GENERATED_DIR LIVE_DIR}"
LIVE_DIR="${2:?Missing LIVE_DIR}"

BACKUP_ROOT="${BACKUP_ROOT:-$(dirname -- "$LIVE_DIR")/benchmark-backups}"
KEEP_BACKUPS="${KEEP_BACKUPS:-10}"
STAMP="$(date +%Y%m%d-%H%M%S)"

log()  { printf '[INFO] %s\n' "$*"; }
ok()   { printf '[ OK ] %s\n' "$*"; }
die()  { printf '[FAIL] %s\n' "$*" >&2; exit 1; }

command -v rsync >/dev/null || die "rsync is required"

GENERATED_DIR="$(cd -- "$GENERATED_DIR" 2>/dev/null && pwd -P)" ||
    die "Generated directory not found: $GENERATED_DIR"

find "$GENERATED_DIR" -mindepth 1 -print -quit | grep -q . ||
    die "Generated directory is empty: $GENERATED_DIR"

LIVE_PARENT="$(dirname -- "$LIVE_DIR")"
LIVE_NAME="$(basename -- "$LIVE_DIR")"

mkdir -p -- "$LIVE_PARENT" "$BACKUP_ROOT"

STAGING="$LIVE_PARENT/.${LIVE_NAME}.new-$$"
OLD="$LIVE_PARENT/.${LIVE_NAME}.old-$$"
BACKUP="$BACKUP_ROOT/${LIVE_NAME}-${STAMP}"
BACKUP_CREATED=0

cleanup() {
    rm -rf -- "$STAGING"
}
trap cleanup EXIT

log "Preparing staging directory: $STAGING"
mkdir -p -- "$STAGING"
rsync -aHAX --delete "$GENERATED_DIR/" "$STAGING/"

find "$STAGING" -mindepth 1 -print -quit | grep -q . ||
    die "Staging directory is empty"

if [[ -e "$LIVE_DIR" ]]; then
    log "Backing up current site to $BACKUP"
    mkdir -p -- "$BACKUP"
    rsync -aHAX "$LIVE_DIR/" "$BACKUP/"
    BACKUP_CREATED=1

    log "Moving current site aside"
    mv -- "$LIVE_DIR" "$OLD"
fi

log "Activating new benchmark site"

if ! mv -- "$STAGING" "$LIVE_DIR"; then
    if [[ -e "$OLD" ]]; then
        mv -- "$OLD" "$LIVE_DIR" || true
    fi

    die "Activation failed; old site restored when possible"
fi

rm -rf -- "$OLD"
trap - EXIT

if [[ "$KEEP_BACKUPS" =~ ^[0-9]+$ ]] && (( KEEP_BACKUPS > 0 )); then
    mapfile -t obsolete < <(
        find "$BACKUP_ROOT" \
            -mindepth 1 -maxdepth 1 \
            -type d \
            -name "${LIVE_NAME}-*" \
            -printf '%T@ %p\n' |
        sort -nr |
        tail -n "+$((KEEP_BACKUPS + 1))" |
        cut -d' ' -f2-
    )

    if (( ${#obsolete[@]} > 0 )); then
        rm -rf -- "${obsolete[@]}"
    fi
fi

ok "Published: $LIVE_DIR"

if (( BACKUP_CREATED == 1 )); then
    ok "Backup:    $BACKUP"
else
    log "No previous live site existed; backup was not needed"
fi

exit 0
