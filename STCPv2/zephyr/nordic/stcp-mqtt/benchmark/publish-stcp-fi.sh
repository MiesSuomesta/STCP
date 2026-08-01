#!/usr/bin/env bash
set -Eeuo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd -- "$SCRIPT_DIR/.." && pwd)"

# Optional local configuration. Command-line options override these values.
CONFIG_FILE="${STCP_PUBLISH_CONFIG:-$SCRIPT_DIR/.stcp-publish.env}"
if [[ -f "$CONFIG_FILE" ]]; then
  # shellcheck disable=SC1090
  source "$CONFIG_FILE"
fi

PUBLISH_HOST="${STCP_PUBLISH_HOST:-}"
PUBLISH_USER="${STCP_PUBLISH_USER:-}"
PUBLISH_PORT="${STCP_PUBLISH_PORT:-22}"
REMOTE_DIR="${STCP_PUBLISH_REMOTE_DIR:-/var/www/stcp.fi/benchmarks/zephyr}"
IDENTITY_FILE="${STCP_PUBLISH_IDENTITY_FILE:-}"
KEEP_RELEASES="${STCP_PUBLISH_KEEP_RELEASES:-10}"
RESULT_DIR=""
LOCAL_TARGET=""
DRY_RUN=0
SKIP_GENERATE=0

usage() {
  cat <<USAGE
Usage: $(basename "$0") [options]

Generates the newest Zephyr benchmark report and publishes it to stcp.fi.

Options:
  --result-dir DIR       Publish a specific benchmark/results/zephyr-* directory
  --host HOST            SSH host (or set STCP_PUBLISH_HOST)
  --user USER            SSH user (or set STCP_PUBLISH_USER)
  --port PORT            SSH port (default: $PUBLISH_PORT)
  --remote-dir DIR       Remote web directory (default: $REMOTE_DIR)
  --identity FILE        SSH private key
  --keep N               Number of archived releases to keep (default: $KEEP_RELEASES)
  --local-target DIR     Publish atomically to a local directory instead of SSH
  --skip-generate        Publish existing benchmark/site/latest without regenerating
  --dry-run              Show rsync operations without modifying the destination
  -h, --help             Show this help

Recommended configuration file:
  $CONFIG_FILE
USAGE
}

fail() { printf '[FAIL] %s\n' "$*" >&2; exit 1; }
info() { printf '[INFO] %s\n' "$*"; }
ok()   { printf '[ OK ] %s\n' "$*"; }

while (($#)); do
  case "$1" in
    --result-dir) RESULT_DIR="${2:?missing directory}"; shift 2 ;;
    --host) PUBLISH_HOST="${2:?missing host}"; shift 2 ;;
    --user) PUBLISH_USER="${2:?missing user}"; shift 2 ;;
    --port) PUBLISH_PORT="${2:?missing port}"; shift 2 ;;
    --remote-dir) REMOTE_DIR="${2:?missing directory}"; shift 2 ;;
    --identity) IDENTITY_FILE="${2:?missing identity file}"; shift 2 ;;
    --keep) KEEP_RELEASES="${2:?missing count}"; shift 2 ;;
    --local-target) LOCAL_TARGET="${2:?missing local target}"; shift 2 ;;
    --skip-generate) SKIP_GENERATE=1; shift ;;
    --dry-run) DRY_RUN=1; shift ;;
    -h|--help) usage; exit 0 ;;
    *) fail "Unknown option: $1" ;;
  esac
done

[[ "$PUBLISH_PORT" =~ ^[0-9]+$ ]] || fail "Invalid SSH port: $PUBLISH_PORT"
[[ "$KEEP_RELEASES" =~ ^[0-9]+$ ]] || fail "Invalid --keep value: $KEEP_RELEASES"

# Resolve the source result set before generation so the release keeps its real name.
if [[ -n "$RESULT_DIR" ]]; then
  RESULT_DIR="$(realpath -m "$RESULT_DIR")"
else
  RESULT_DIR="$(find "$SCRIPT_DIR/results" -mindepth 1 -maxdepth 1 -type d -name 'zephyr-*' -printf '%T@ %p\n' 2>/dev/null | sort -nr | head -n1 | cut -d' ' -f2-)"
fi
[[ -n "$RESULT_DIR" && -d "$RESULT_DIR" ]] || fail "No benchmark/results/zephyr-* result directory found"
SOURCE_RELEASE_NAME="$(basename "$RESULT_DIR")"

if (( ! SKIP_GENERATE )); then
  info "Generating benchmark report from $RESULT_DIR"
  python3 "$SCRIPT_DIR/generate-report.py" "$RESULT_DIR"
fi

SITE_DIR="$SCRIPT_DIR/site/latest"
[[ -f "$SITE_DIR/index.html" ]] || fail "Missing $SITE_DIR/index.html"
[[ -f "$SITE_DIR/data.js" ]] || fail "Missing $SITE_DIR/data.js"
[[ -f "$SITE_DIR/raw/pipeline-summary.json" ]] || fail "Missing raw/pipeline-summary.json"

# Read and validate the report metadata before publication.
readarray -t META < <(python3 - "$SITE_DIR/raw/pipeline-summary.json" "$SOURCE_RELEASE_NAME" <<'PY'
import json, pathlib, re, sys
p = pathlib.Path(sys.argv[1])
d = json.loads(p.read_text())
total = int(d.get("cases_total", 0))
passed = int(d.get("cases_passed", 0))
failed = int(d.get("cases_failed", total - passed))
if total <= 0:
    raise SystemExit("pipeline-summary contains no cases")
if failed != 0 or passed != total:
    raise SystemExit(f"refusing to publish incomplete result: passed={passed}, failed={failed}, total={total}")
name = re.sub(r"[^A-Za-z0-9._-]", "-", sys.argv[2])
print(name)
print(total)
print(passed)
print(d.get("platform", "unknown"))
print(d.get("carrier", "unknown"))
PY
) || fail "Report metadata validation failed"

RELEASE_NAME="${META[0]}"
CASES_TOTAL="${META[1]}"
PLATFORM="${META[3]}"
CARRIER="${META[4]}"
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
WORK_DIR="$(mktemp -d "${TMPDIR:-/tmp}/stcp-publish.XXXXXX")"
trap 'rm -rf "$WORK_DIR"' EXIT
STAGE_DIR="$WORK_DIR/$RELEASE_NAME"
mkdir -p "$STAGE_DIR"

info "Preparing release $RELEASE_NAME ($CASES_TOTAL cases, $PLATFORM, $CARRIER)"
rsync -a --delete "$SITE_DIR/" "$STAGE_DIR/"

python3 - "$STAGE_DIR" "$RELEASE_NAME" "$PLATFORM" "$CARRIER" "$CASES_TOTAL" <<'PY'
import datetime, hashlib, json, pathlib, sys
root = pathlib.Path(sys.argv[1])
files = []
for p in sorted(root.rglob('*')):
    if p.is_file():
        files.append({
            "path": p.relative_to(root).as_posix(),
            "bytes": p.stat().st_size,
            "sha256": hashlib.sha256(p.read_bytes()).hexdigest(),
        })
manifest = {
    "schema_version": 1,
    "release": sys.argv[2],
    "platform": sys.argv[3],
    "carrier": sys.argv[4],
    "cases_total": int(sys.argv[5]),
    "published_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
    "files": files,
}
(root / "publish-manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
PY

cat > "$WORK_DIR/index.html" <<'HTML'
<!doctype html><html lang="fi"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><meta http-equiv="refresh" content="0;url=latest/"><title>STCPv2 benchmark</title></head><body><p><a href="latest/">Avaa uusin STCPv2 benchmark</a></p></body></html>
HTML

RSYNC_ARGS=(-a --delete --delay-updates --human-readable)
(( DRY_RUN )) && RSYNC_ARGS+=(--dry-run --itemize-changes)

publish_local() {
  local target="$1" incoming="$1/.incoming-$STAMP" releases="$1/releases"
  info "Publishing locally to $target"
  if (( DRY_RUN )); then
    rsync "${RSYNC_ARGS[@]}" "$STAGE_DIR/" "$incoming/"
    return
  fi
  mkdir -p "$target" "$releases"
  rm -rf "$incoming"
  rsync "${RSYNC_ARGS[@]}" "$STAGE_DIR/" "$incoming/"
  [[ -f "$incoming/index.html" && -f "$incoming/data.js" ]] || fail "Local incoming release validation failed"
  rm -rf "$releases/$RELEASE_NAME"
  mv "$incoming" "$releases/$RELEASE_NAME"
  rm -rf "$target/.latest.previous"
  [[ ! -e "$target/latest" ]] || mv "$target/latest" "$target/.latest.previous"
  cp -a "$releases/$RELEASE_NAME" "$target/latest"
  cp "$WORK_DIR/index.html" "$target/index.html"
  rm -rf "$target/.latest.previous"
  if (( KEEP_RELEASES > 0 )); then
    mapfile -t old < <(find "$releases" -mindepth 1 -maxdepth 1 -type d -printf '%T@ %p\n' | sort -nr | tail -n +$((KEEP_RELEASES + 1)) | cut -d' ' -f2-)
    ((${#old[@]} == 0)) || rm -rf -- "${old[@]}"
  fi
}

publish_remote() {
  [[ -n "$PUBLISH_HOST" ]] || fail "Set --host or STCP_PUBLISH_HOST"
  [[ -n "$PUBLISH_USER" ]] || fail "Set --user or STCP_PUBLISH_USER"
  local destination="$PUBLISH_USER@$PUBLISH_HOST"
  local incoming="$REMOTE_DIR/.incoming-$STAMP"
  local ssh=(ssh -p "$PUBLISH_PORT" -o BatchMode=yes -o ConnectTimeout=15)
  local rsync_ssh="ssh -p $PUBLISH_PORT -o BatchMode=yes -o ConnectTimeout=15"
  if [[ -n "$IDENTITY_FILE" ]]; then
    [[ -f "$IDENTITY_FILE" ]] || fail "SSH identity not found: $IDENTITY_FILE"
    ssh+=(-i "$IDENTITY_FILE")
    rsync_ssh+=" -i $(printf '%q' "$IDENTITY_FILE")"
  fi

  info "Checking SSH connection to $destination"
  "${ssh[@]}" "$destination" "mkdir -p '$REMOTE_DIR' '$REMOTE_DIR/releases'"

  info "Uploading release to $destination:$incoming"
  rsync "${RSYNC_ARGS[@]}" -e "$rsync_ssh" "$STAGE_DIR/" "$destination:$incoming/"
  if (( DRY_RUN )); then
    info "Dry run complete; remote activation skipped"
    return
  fi

  # Upload landing page separately.
  rsync -a -e "$rsync_ssh" "$WORK_DIR/index.html" "$destination:$REMOTE_DIR/.index.incoming"

  info "Activating release atomically"
  "${ssh[@]}" "$destination" bash -s -- "$REMOTE_DIR" "$incoming" "$RELEASE_NAME" "$KEEP_RELEASES" <<'REMOTE'
set -Eeuo pipefail
base="$1"; incoming="$2"; release="$3"; keep="$4"
[[ -f "$incoming/index.html" && -f "$incoming/data.js" && -f "$incoming/publish-manifest.json" ]] || {
  echo "incoming release validation failed" >&2; exit 1;
}
rm -rf "$base/releases/$release"
mv "$incoming" "$base/releases/$release"
rm -rf "$base/.latest.next" "$base/.latest.previous"
cp -a "$base/releases/$release" "$base/.latest.next"
if [[ -e "$base/latest" ]]; then mv "$base/latest" "$base/.latest.previous"; fi
mv "$base/.latest.next" "$base/latest"
mv "$base/.index.incoming" "$base/index.html"
rm -rf "$base/.latest.previous"
if [[ "$keep" =~ ^[0-9]+$ ]] && (( keep > 0 )); then
  mapfile -t old < <(find "$base/releases" -mindepth 1 -maxdepth 1 -type d -printf '%T@ %p\n' | sort -nr | tail -n +$((keep + 1)) | cut -d' ' -f2-)
  ((${#old[@]} == 0)) || rm -rf -- "${old[@]}"
fi
REMOTE
}

if [[ -n "$LOCAL_TARGET" ]]; then
  publish_local "$(realpath -m "$LOCAL_TARGET")"
else
  publish_remote
fi

ok "Published $RELEASE_NAME"
if [[ -n "$LOCAL_TARGET" ]]; then
  printf 'Open: file://%s/latest/index.html\n' "$(realpath -m "$LOCAL_TARGET")"
else
  printf 'URL path: /benchmarks/zephyr/\n'
fi
