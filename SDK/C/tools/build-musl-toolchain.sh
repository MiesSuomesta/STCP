#!/bin/bash
set -euo pipefail

source config_build_variables

export PATH="$TOOLCHAIN/bin/:$PATH"

# 1. Lataa musl-cross-make
if [ ! -d musl-cross-make ]; then
  git clone https://github.com/richfelker/musl-cross-make.git
fi
cd musl-cross-make

# 2. Luo config.mak oikeilla asetuksilla
export CFLAGS="-fPIC"
export LDFLAGS="--static"


# Konfiguroi MUSL + staattinen buildi

cat > config.mak <<EOF
TARGET = $TARGET
OUTPUT = $INSTALL_DIR
GCC_CONFIG += --enable-languages=c,c++
GCC_CONFIG += --disable-lto
EOF

# 3. Käännä
CFLAGS_FOR_TARGET="-O2 -fPIC"
CXXFLAGS_FOR_TARGET="-O2 -fPIC"

if [ "x${1:-dirty}" == "xclean" ]; then
	CFLAGS_FOR_TARGET="$CFLAGS_FOR_TARGET" CXXFLAGS_FOR_TARGET="$CXXFLAGS_FOR_TARGET" \
	  PATH="$PATH" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" make clean
else
	echo "⚠️  Clean skipped.."
fi

rm -rf "$INSTALL_DIR"


CFLAGS_FOR_TARGET="$CFLAGS_FOR_TARGET" CXXFLAGS_FOR_TARGET="$CXXFLAGS_FOR_TARGET" \
  PATH="$PATH" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" make -j$(nproc) TARGET=$TARGET


CFLAGS_FOR_TARGET="$CFLAGS_FOR_TARGET" CXXFLAGS_FOR_TARGET="$CXXFLAGS_FOR_TARGET" \
  PATH="$PATH" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" make -j$(nproc) install

echo "✅ Compile options:"
echo "     CFLAGS              : $CFLAGS"
echo "     CXXFLAGS            : $CXXFLAGS"
echo "     CFLAGS (target)     : $CFLAGS_FOR_TARGET"
echo "     CXXFLAGS (target)   : $CXXFLAGS_FOR_TARGET"
echo "     LDFLAGS             : $LDFLAGS"

