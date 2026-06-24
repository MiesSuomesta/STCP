#!/usr/bin/env bash
set -euxo pipefail

. config_openssl
PREFIX="$OPENSSL_PREFIX"

# Lataa lähdekoodi
curl -LO https://www.openssl.org/source/openssl-${OPENSSL_VER}.tar.gz
rm -rf openssl-${OPENSSL_VER}
tar xf openssl-${OPENSSL_VER}.tar.gz
cd openssl-${OPENSSL_VER}

# Aseta ympäristö
export CC="$CC_PATH"
export CFLAGS="-fPIC -DOPENSSL_NO_GETHOSTBYNAME"
export LDFLAGS="--static"


# Konfiguroi MUSL + staattinen buildi
CC="$CC" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" ./Configure linux-x86_64 no-shared no-dso no-async no-ssl2 no-ssl3 \
    no-comp no-zlib no-weak-ssl-ciphers no-threads no-dynamic-engine \
  --prefix=$PREFIX \
  --openssldir=$PREFIX/ssl -fPIC -DOPENSSL_NO_GETHOSTBYNAME

# Käännä
CC="$CC" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" make clean || true
CC="$CC" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" make -j$(nproc) 2>&1 | tee openssl_compile.log
CC="$CC" CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" make install_sw

echo "✅ OpenSSL ready at: $PREFIX"
