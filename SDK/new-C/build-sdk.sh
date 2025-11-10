#!/usr/bin/env bash
set -euo pipefail

LOGFILE="/tmp/full-build-log.$$.txt"
PROJECT_ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$PROJECT_ROOT"

source tools/config_build_variables

banner() {
    B="$1"
    shift

    echo "$B ================================================================================================"
    for l in "$@"
    do
        echo "   $l"
    done
    echo "   ================================================================================================"
}

doOnTrap() {
    banner "💥" "BANG! Soething went wrong!"
    cp "$LOGFILE" full-build.log
    tail -n 15 full-build.log

}
trap "doOnTrap" ERR


banner "🚀" "STCP SDK: Full rebuild stared at $(date)" | tee -a "$LOGFILE"
echo   "📁" "Project root: $PROJECT_ROOT" | tee -a "$LOGFILE"

(
	# Siivotaan
	banner "🧹" "Cleaning everything + lock reset"
	bash clean-all.sh

	#banner "🧹" "Automatic fixup...."
	#for crate in stcpclientlib stcpserverlib stcp_client_cwrapper_lib stcp_server_cwrapper_lib; do
	#	cargo fix --lib -p "$crate" --allow-dirty --allow-staged --target x86_64-unknown-linux-musl
	#done

	# Rakenna kaikki --workspace flagilla varmistaen riippuvuuksien näkyvyys
	for crate in stcpclientlib stcpserverlib stcp_client_cwrapper_lib stcp_server_cwrapper_lib; do
	    banner "📦" "Building crate $crate ................."
	    cargo build --manifest-path "$PROJECT_ROOT/Cargo.toml" --workspace -p "$crate" --release --target="$TARGET"
	done
) | tee -a "$LOGFILE"

banner "📦" "Whole project built at $(date)"

banner "🧪" "Generating SDK package ..."
source generate_sdk.sh 

banner "🧪" "Testing SDK for all needed symbols..."
pushd sdk > /dev/null

rm -rf build || true
cmake -B build -DTESTING=1
cmake --build build
rm -rf build || true

popd > /dev/null

banner "✅" "SDK Complete! Everything worked as planned. SDK built at $(date)"
