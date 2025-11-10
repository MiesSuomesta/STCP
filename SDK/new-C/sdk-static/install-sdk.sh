#!/usr/bin/env bash
set -euo pipefail

PREFIX="${1:-/usr/local}"

echo "📦 Installing STCP SDK with prefix: $PREFIX"

install -d "${PREFIX}/include"
install -d "${PREFIX}/lib"
install -d "${PREFIX}/lib/pkgconfig"

cp -v include/*.h "${PREFIX}/include/"
cp -v lib/*.a "${PREFIX}/lib/"
cp -v pkgconfig/stcp-sdk.pc "${PREFIX}/lib/pkgconfig/"

echo "✅ Installation complete!"
echo ""
echo "Use with pkg-config:"
echo "  PKG_CONFIG_PATH=${PREFIX}/lib/pkgconfig pkg-config --cflags --libs stcp-sdk"
