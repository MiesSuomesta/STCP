#!/bin/bash
NM="$HOME/git/stcp/ports/dect-nr+/musl-cross-compiled-libs/bin/x86_64-linux-musl-nm"
OUT_LIB="$1"

if [ -f "$OUT_LIB" ]; then
  echo "✅ Kirjasto luotu onnistuneesti: $OUT_LIB"
else
  echo "❌ Kirjastoa ei syntynyt! Tarkista edeltävät vaiheet."
  exit 1
fi

UDEF=$($NM -g "$OUT_LIB" |grep " U " | sort | uniq | wc -l | awk '{print $1}')

if [ $UDEF -gt 0  ]
then
        echo "❌ Kirjastossa on määrittämättömia symbooleita..."
        $NM -g "$OUT_LIB" |grep " U "  | sed 's,^,nm -g with grep U   |,' >  lib_analysis.txt
        $NM -nu "$OUT_LIB" | c++filt   | sed 's,^,nm -nu with c++filt |,' >> lib_analysis.txt
        $NM "$OUT_LIB" | c++filt       | sed 's,^,nm with c++filt     |,' >> lib_analysis.txt
	echo "Undefined symbooleita: $UDEF"
	exit 1
else
        echo "✅ Kirjasto tarkistus: OK"
	exit 0
fi

