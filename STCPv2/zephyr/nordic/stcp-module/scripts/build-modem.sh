#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
BOARD="${BOARD:-nrf9151dk/nrf9151}"
APP="${APP:-$ROOT/samples/stcp_c_port}"
BUILD_DIR="${BUILD_DIR:-$ROOT/build-nrf9151}"
SDK_DIR="${ZEPHYR_SDK_INSTALL_DIR:-$HOME/zephyr-sdk-1.0.1}"
PRISTINE="${PRISTINE:-always}"
USE_SYSBUILD="${USE_SYSBUILD:-0}"

usage() {
    cat <<USAGE
Usage: $(basename "$0") [--board BOARD] [--build-dir DIR] [--app DIR]
                          [--sdk-dir DIR] [--sysbuild] [--no-pristine]
                          [-- EXTRA_CMAKE_ARGS...]

Build the STCP Zephyr application for the nRF9151 application core.
The nRF91 modem firmware itself is prebuilt by Nordic and is not compiled here.

Environment overrides:
  BOARD, APP, BUILD_DIR, ZEPHYR_SDK_INSTALL_DIR, PRISTINE, USE_SYSBUILD
USAGE
}

extra_cmake=()
while (($#)); do
    case "$1" in
        --board) BOARD="$2"; shift 2 ;;
        --build-dir) BUILD_DIR="$2"; shift 2 ;;
        --app) APP="$2"; shift 2 ;;
        --sdk-dir) SDK_DIR="$2"; shift 2 ;;
        --sysbuild) USE_SYSBUILD=1; shift ;;
        --no-pristine) PRISTINE=auto; shift ;;
        -h|--help) usage; exit 0 ;;
        --) shift; extra_cmake+=("$@"); break ;;
        *) echo "Unknown argument: $1" >&2; usage >&2; exit 2 ;;
    esac
done

command -v west >/dev/null 2>&1 || {
    echo "Error: west is not on PATH. Activate the Nordic virtual environment first." >&2
    exit 1
}

[[ -f "$APP/CMakeLists.txt" ]] || {
    echo "Error: Zephyr application not found: $APP" >&2
    exit 1
}

[[ -f "$ROOT/zephyr/module.yml" ]] || {
    echo "Error: STCP OOT module root is invalid: $ROOT" >&2
    exit 1
}

if [[ ! -d "$SDK_DIR" ]]; then
    echo "Error: Zephyr SDK directory not found: $SDK_DIR" >&2
    echo "Set ZEPHYR_SDK_INSTALL_DIR or pass --sdk-dir." >&2
    exit 1
fi

export ZEPHYR_TOOLCHAIN_VARIANT="${ZEPHYR_TOOLCHAIN_VARIANT:-zephyr}"
export ZEPHYR_SDK_INSTALL_DIR="$SDK_DIR"

west_args=(build -p "$PRISTINE" -d "$BUILD_DIR" -b "$BOARD")
if [[ "$USE_SYSBUILD" == "1" ]]; then
    west_args+=(--sysbuild)
else
    west_args+=(--no-sysbuild)
fi
west_args+=("$APP" -- "-DZEPHYR_EXTRA_MODULES=$ROOT")
west_args+=("${extra_cmake[@]}")

echo "Building STCP for $BOARD"
echo "Application: $APP"
echo "Build dir:  $BUILD_DIR"
echo "SDK:        $ZEPHYR_SDK_INSTALL_DIR"
west "${west_args[@]}"

echo
echo "Build complete. Main artifacts:"
for artifact in \
    "$BUILD_DIR/zephyr/zephyr.elf" \
    "$BUILD_DIR/zephyr/zephyr.hex" \
    "$BUILD_DIR/zephyr/zephyr.bin"; do
    [[ -f "$artifact" ]] && printf '  %s\n' "$artifact"
done
