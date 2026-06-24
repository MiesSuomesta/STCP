#!/bin/bash
set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "$0/")/.." && pwd)"

cd "$PROJECT_ROOT"/tools

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

sudo apt update || true
banner "📦" "Installing essentials...."
#sudo apt install build-essential wget curl git cmake pkg-config libssl-dev rustup
banner "✅" "Installed essential: OK"

export INSTALL_DIR="${PROJECT_ROOT}/tools/musl-cross-compiled-libs"
export TARGET=x86_64-linux-musl
export MUSL_HOST=x86_64-unknown-linux-gnu
export MUSL_TARGET=x86_64-unknown-linux-musl


banner "📦" "Installing toolchain...." \
            "    For host system    : ${MUSL_HOST}" \
            "    For target system  : ${MUSL_TARGET}" \
            "    Toolchain location : $INSTALL_DIR"

source build-musl-toolchain.sh

banner "✅" "Installed toolchain: OK"

export CC=$TOOLCHAIN/bin/${TARGET}-gcc
export CXX=$TOOLCHAIN/bin/${TARGET}-g++
export LD=$TOOLCHAIN/bin/${TARGET}-ld
export AR=$TOOLCHAIN/bin/${TARGET}-ar
export RANLIB=$TOOLCHAIN/bin/${TARGET}-ranlib
export NM=$TOOLCHAIN/bin/${TARGET}-nm

banner "📦" "Installied toolchain, components:" \
            "    CC         : $CC" \
            "    CXX        : $CXX" \
            "    LD         : $LD" \
            "    AR         : $AR" \
            "    RANLIB     : $RANLIB" \
            "    NM         : $NM"


export PATH="$HOME/.cargo/bin:$PATH"

banner "📦" "Installing dependencies...."
bash "$DEP_MAIN_DIR/build-all.sh"
banner "✅" "Installed dependencies....OK"

# Tarvittavat ympäristömuuttujat
export RUSTFLAGS="-C target-feature=+crt-static"
export OPENSSL_STATIC=1
export PKG_CONFIG_ALLOW_CROSS=1
export CARGO_TARGET_X86_64_UNKNOWN_LINUX_MUSL_LINKER=musl-gcc


# Nightly + MUSL-target + build std
#banner "📦" "Installing rustup ${MUSL_HOST} toolchain for target ${MUSL_TARGET}"
#rustup uninstall nightly
#rustup install nightly
#rustup component add rust-src --toolchain nightly-${MUSL_HOST}
#rustup target add ${MUSL_TARGET} --toolchain nightly
#banner "✅" "Installed rustup ${MUSL_HOST} toolchain: OK for ${MUSL_TARGET} .."

# Asennetaan cbindgen isäntätoolchainilla
banner "📦" "Installing cbindgen...."
unset CC CXX LD AR RANLIB NM CFLAGS LDFLAGS RUSTFLAGS CARGO_TARGET_X86_64_UNKNOWN_LINUX_MUSL_LINKER
cargo uninstall cbindgen || true
cargo install -f cbindgen 
banner "✅" "Installed cbindgen....OK"





banner "✅" "Setup complete. You are ready to build STCP SDK!"
