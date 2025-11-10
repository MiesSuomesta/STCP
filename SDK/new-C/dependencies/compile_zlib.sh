#!/bin/bash
set -euo pipefail

. config_zlib
PREFIX="$ZLIB_PREFIX"

curl -LO https://zlib.net/zlib-$ZLIB_VER.tar.gz
rm -rf zlib-$ZLIB_VER
tar zxvf zlib-$ZLIB_VER.tar.gz
cd zlib-$ZLIB_VER

# Aseta ympäristö
export CC=$CC_PATH
export CFLAGS="-fPIC"
export LDFLAGS="--static"

CC="$CC" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" ./configure --prefix=$PREFIX --static
CC="$CC" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" make -j$(nproc) 2>&1 | tee zlib_compile.log
CC="$CC" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" make install

echo "✅ ZLIB ready at: $PREFIX"
