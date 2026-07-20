#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build-nrf9151}"
RUNNER="${RUNNER:-nrfutil}"
SERIAL="${SERIAL:-}"
RECOVER=0
ERASE=0
NO_REBUILD=0

usage() {
    cat <<USAGE
Usage: $(basename "$0") [--build-dir DIR] [--runner nrfutil|jlink]
                          [--serial SERIAL] [--recover] [--erase]
                          [--no-rebuild] [-- EXTRA_WEST_FLASH_ARGS...]

Flash the nRF9151 application image produced by build-modem.sh.
--recover erases the target and removes AP-Protect before flashing.
USAGE
}

extra_args=()
while (($#)); do
    case "$1" in
        --build-dir) BUILD_DIR="$2"; shift 2 ;;
        --runner) RUNNER="$2"; shift 2 ;;
        --serial) SERIAL="$2"; shift 2 ;;
        --recover) RECOVER=1; shift ;;
        --erase) ERASE=1; shift ;;
        --no-rebuild) NO_REBUILD=1; shift ;;
        -h|--help) usage; exit 0 ;;
        --) shift; extra_args+=("$@"); break ;;
        *) echo "Unknown argument: $1" >&2; usage >&2; exit 2 ;;
    esac
done

command -v west >/dev/null 2>&1 || {
    echo "Error: west is not on PATH. Activate the Nordic virtual environment first." >&2
    exit 1
}

[[ -f "$BUILD_DIR/zephyr/zephyr.hex" ]] || {
    echo "Error: firmware image not found: $BUILD_DIR/zephyr/zephyr.hex" >&2
    echo "Run scripts/build-modem.sh first." >&2
    exit 1
}

case "$RUNNER" in
    nrfutil)
        command -v nrfutil >/dev/null 2>&1 || {
            echo "Error: nrfutil is not on PATH." >&2
            exit 1
        }
        ;;
    jlink)
        command -v JLinkExe >/dev/null 2>&1 || {
            echo "Error: JLinkExe is not on PATH." >&2
            exit 1
        }
        ;;
    *) echo "Error: unsupported runner: $RUNNER" >&2; exit 2 ;;
esac

args=(flash -d "$BUILD_DIR" -r "$RUNNER")
[[ "$NO_REBUILD" == "1" ]] && args+=(--skip-rebuild)
[[ "$RECOVER" == "1" ]] && args+=(--recover)
[[ "$ERASE" == "1" ]] && args+=(--erase)
[[ -n "$SERIAL" ]] && args+=(--dev-id "$SERIAL")
args+=("${extra_args[@]}")

echo "Flashing: $BUILD_DIR/zephyr/zephyr.hex"
echo "Runner:   $RUNNER"
[[ -n "$SERIAL" ]] && echo "Probe:    $SERIAL"
west "${args[@]}"
