#!/bin/bash
set -euo pipefail

MY_DIR=$(dirname "$0")

PROJECT_ROOT="$(cd "${MY_DIR}/.." && pwd)"

echo "Project root: ${PROJECT_ROOT} ...."

source "$PROJECT_ROOT"/tools/config_build_variables

MY_ROOT="$PROJECT_ROOT/dependencies"

pushd "$MY_ROOT"
	source compile_zlib.sh
popd
