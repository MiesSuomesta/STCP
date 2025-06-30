#!/bin/bash
set -e

DT=$(date +"%Y.%m.%d_%H-%M-%S" )

# Määrittele nimi
ZIPNAME="stcp-project-${DT}.zip"
ROOTDIR="."

# Siirry projektin juureen
cd "$(dirname "$0")/.."

# Poistetaan mahdollinen aiempi zip
rm -f "$ZIPNAME"

# Pakkaa mutta sulkee pois build-tiedostot
zip -r "$ZIPNAME" "$ROOTDIR" \
    -x "$ROOTDIR/common/musl-cross-make/*" \
    -x "$ROOTDIR/dependencies/*" \
    -x "$ROOTDIR/tools/musl-*/*" \
    -x "$ROOTDIR/target*/*" \
    -x "$ROOTDIR/**/target*/*" \
    -x "$ROOTDIR/**/*.o" \
    -x "$ROOTDIR/**/*.a" \
    -x "$ROOTDIR/**/*.rmeta" \
    -x "$ROOTDIR/**/*.rlib" \
    -x "$ROOTDIR/**/*.dSYM/*" \
    -x "$ROOTDIR/**/*.log" \
    -x "$ROOTDIR/**/libstcpServer*.a" \
    -x "$ROOTDIR/**/libstcp*.a" \
    -x "$ROOTDIR/**/build/*"

echo "✅ Pakkaus valmis: $ZIPNAME"
