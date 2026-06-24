#!/bin/bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

source "$PROJECT_ROOT"/tools/config_build_variables

MY_ROOT="$(cd "$(dirname "$0")" && pwd)"

pushd "$MY_ROOT"
	source compile_open_ssl.sh 
popd

pushd "$MY_ROOT"
	source compile_zlib.sh
popd
