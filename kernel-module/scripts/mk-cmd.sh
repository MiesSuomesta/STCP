#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/" && pwd)"
KMOD_DIR="${ROOT}/../kmod"
RUST_TARGET_DIR="${ROOT}/../rust/target/mun/release"

LIB_NAME="libthe_rust_implementation.a"
LIB_PATH="${RUST_TARGET_DIR}/${LIB_NAME}"

if [[ ! -f "${LIB_PATH}" ]]; then
    echo "ERROR: Rust staticlib not found: ${LIB_PATH}" >&2
    exit 1
fi

OBJ_PATH="${KMOD_DIR}/the_rust_implementation.o"
CMD_PATH="${KMOD_DIR}/.the_rust_implementation.o.cmd"

$ROOT/rust-ar-to-o.sh "${LIB_PATH}" "${OBJ_PATH}" "${CMD_PATH}"

