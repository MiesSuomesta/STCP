#!/bin/bash
set -euo pipefail

LOGFILE="/tmp/full-build-log.$$.txt"
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_ROOT"

# 🔧 Lataa ympäristömuuttujat ja kirjastopolut
source ./build_environment.env

runCargo() {
    echo "▶️  cargo $*" 2>&1 | tee -a "$LOGFILE"
    echo "📂 Run at: $(pwd)" 2>&1 | tee -a "$LOGFILE"
    echo "⚙️  RUSTFLAGS = $RUSTFLAGS" | tee -a "$LOGFILE"
    echo "⚙️  CFLAGS = $CFLAGS" | tee -a "$LOGFILE"

	PATH="$PATH"               \
	CC="$TOOLCHAIN_CC"         \
	LD="$TOOLCHAIN_LD"         \
	NM="$TOOLCHAIN_NM"         \
	RANLIB="$TOOLCHAIN_RANLIB" \
	AR="$TOOLCHAIN_AR"         \
	CFLAGS="${CFLAGS}"         \
	LDFLAGS="${LDFLAGS}"       \
	TARGET="$TARGET"           \
	RUSTFLAGS="$RUSTFLAGS"     \
		cargo "$@" --manifest-path "$PROJECT_ROOT/Cargo.toml" 2>&1 | tee -a "$LOGFILE"
}

cd "$1"
shift
runCargo "$@"
