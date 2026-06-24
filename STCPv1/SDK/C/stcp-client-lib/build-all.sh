#!/bin/bash 
set -euo pipefail
cln=${1:-nope}
checks=${2:-nope}
BLOG=/tmp/build-log.$$.txt

source prebuild-config.shinc  2>&1 | tee -a $BLOG

toggle_target_override() {
    local mode="$1"  # "disable" tai "restore"
    local cfg_file=".cargo/config.toml"
    local bak_file=".cargo/config.toml.bak"

    if [[ "$mode" == "disable" ]]; then
        if grep -q "^\s*target\s*=" "$cfg_file"; then
            echo "🔧 Disabling 'target =' line in $cfg_file"
            cp "$cfg_file" "$bak_file"
            sed -i 's/^\(\s*target\s*=.*\)/# \1/' "$cfg_file"
        else
            echo "✅ No 'target =' line found to disable"
        fi
    elif [[ "$mode" == "restore" ]]; then
        if [[ -f "$bak_file" ]]; then
            echo "♻️  Restoring original $cfg_file"
            mv "$bak_file" "$cfg_file"
        else
            echo "⚠️  No backup file found to restore"
        fi
    else
        echo "Usage: toggle_target_override disable|restore"
        return 1
    fi
}

runCargoInRoot() {
    local root_path="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
    (
        cd "$root_path"
	toggle_target_override disable
        echo "🔧 (from root) cargo +nightly $*"
        cargo +nightly "$@"
	toggle_target_override restore
    )
}

setupCargo() {
        mkdir -p .cargo || true
        cp -av $1/* .cargo/ || true
}

doOnTrap() {
        echo "💥 BANG! Somethign wrong at $1"
        cp $BLOG build-log.txt
        exit 1
}

trap "doOnTrap ${LINENRO:-0}" ERR

doCleanCargo() {
        (
                cd $1
                echo "🔄 Cleaning $* (at $PWD) ...."
                cargo clean
                rm -rf target Cargo.lock
        ) 2>&1 | tee -a $BLOG
 
}

[ "x$cln" == "xclean" ] && (

        doCleanCargo stcp-client-lib/rust-client-lib "RUST client lib"

        doCleanCargo stcp-client-lib/rust-c-wrapper "RUST C wrapper lib"

        echo "🔄 Cleaning main build (at $PWD) ...."
        rm -rf build  
)  2>&1 | tee -a $BLOG


 
(
        echo "🧱 Build: RUST client lib ......"
        source prebuild-config.shinc 
        setupCargo .cargo
	echo "🔎 Cargo check enabled: $checks.................."
	[ "x$checks" != "xnope" ] && (
        	PATH="$PATH" CC="$TOOLCHAIN_CC" LD="$TOOLCHAIN_LD" RANLIB="$TOOLCHAIN_RANLIB" AR="$TOOLCHAIN_AR" \
                	CFLAGS="${CFLAGS:--fPIC}" LDFLAGS="${LDFLAGS:---static}" TARGET="$TARGET" RUSTFLAGS="$RUSTFLAGS" \
                	runCargoInRoot check -p stcpclientlib --target x86_64-unknown-linux-gnu

		echo "🔎 Cargo check stcpclientlib OK.................."

        	PATH="$PATH" CC="$TOOLCHAIN_CC" LD="$TOOLCHAIN_LD" RANLIB="$TOOLCHAIN_RANLIB" AR="$TOOLCHAIN_AR" \
                	CFLAGS="${CFLAGS:--fPIC}" LDFLAGS="${LDFLAGS:---static}" TARGET="$TARGET" RUSTFLAGS="$RUSTFLAGS" \
                	runCargoInRoot check -p stcp_client_cwrapper_lib --target x86_64-unknown-linux-gnu

		echo "🔎 Cargo check stcp_client_cwrapper_lib OK.................."
	)

        PATH="$PATH" CC="$TOOLCHAIN_CC" LD="$TOOLCHAIN_LD" RANLIB="$TOOLCHAIN_RANLIB" AR="$TOOLCHAIN_AR" \
                CFLAGS="${CFLAGS:--fPIC}" LDFLAGS="${LDFLAGS:---static}" TARGET="$TARGET" RUSTFLAGS="$RUSTFLAGS" \
                cargo +nightly build --release --target x86_64-unknown-linux-musl

        echo "🧱 Build: RUST client lib.... DONE"
) 2>&1 | tee -a $BLOG
 
 
(
        source prebuild-config.shinc 
        mkdir -p  build && cd build
        echo "🧱 Build: Calling CMake..."

        PATH="$PATH" CC="$TOOLCHAIN_CC" LD="$TOOLCHAIN_LD" RANLIB="$TOOLCHAIN_RANLIB" AR="$TOOLCHAIN_AR" \
                CFLAGS="${CFLAGS:--fPIC}" LDFLAGS="${LDFLAGS:---static}" TARGET="$TARGET" RUSTFLAGS="$RUSTFLAGS" \
                cmake ..

        echo "🧱 Build: Calling CMake...DONE"
        echo "🧱 Build: Building...."
        PATH="$PATH" CC="$TOOLCHAIN_CC" LD="$TOOLCHAIN_LD" RANLIB="$TOOLCHAIN_RANLIB" AR="$TOOLCHAIN_AR" \
                CFLAGS="${CFLAGS:--fPIC}" LDFLAGS="${LDFLAGS:---static}" TARGET="$TARGET" RUSTFLAGS="$RUSTFLAGS" \
                 make -j$(nproc)
        echo "🧱 Build: Building....DONE"
        ls -l
) 2>&1 | tee -a $BLOG

(
        source prebuild-config.shinc 
        for tname in ./build/lib/libstcp_client_cwrapper_lib.a ./build/stcp_client
        do
                echo "🔍 Testing wrapper functions from $tname ............."
                bash check_wrapper_exports.sh \
                  $tname \
                  build/include/stcp_client_cwrapper_lib.h
                echo "🔍 Testing wrapper functions.............DONE"
        done
) 2>&1 | tee -a $BLOG

cp $BLOG build-log.txt
