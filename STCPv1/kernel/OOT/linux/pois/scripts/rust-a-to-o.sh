#!/usr/bin/env bash
# Yhdistää Rustin staticlib (.a) → yhdeksi .o-tiedostoksi kerneliä varten.
# Käyttö:
#   rust-ar-to-o.sh <INPUT.a> <OUTPUT.o> <CMD-FILE> [nm-bin] [ld-bin]
#
# Esim:
#   rust-ar-to-o.sh rust/target/mun/release/libthe_stcp_kernel_module.a \
#                   kmod/the_stcp_kernel_module.o \
#                   kmod/.the_stcp_kernel_module.o.cmd \
#                   nm \
#                   ld.bfd

set -euo pipefail

IN="${1:?need input .a}"
OUT="${2:?need output .o}"
CMD="${3:?need cmd file}"
NM="${4:-nm}"
LD="${5:-ld.bfd}"

echo "[stcp] Input:  $IN"
echo "[stcp] Output: $OUT"
echo "[stcp] .cmd:   $CMD"
echo "[stcp] nm:     $NM"
echo "[stcp] ld:     $LD"

if [ ! -f "$IN" ]; then
    echo "[stcp] ERROR: input archive '$IN' not found" >&2
    exit 1
fi

# Varsinainen linkkaus:
# --whole-archive: vedä kaikki .o:t mukaan, ettei mikään Rust-symboli tipu pois
# -r: tee relocatable .o, jota kerneli voi linkittää moduuliin
LINK_CMD=( "$LD" -r -m elf_x86_64
           --whole-archive "$IN" --no-whole-archive
           -o "$OUT" )

echo "[stcp] Linking → ${LINK_CMD[*]}"
"${LINK_CMD[@]}"

# Pieni tarkistus: löytyykö ainakin yksi Rust-exportti?
# (ei pakollinen buildille; jos ei löydy, vain varoitus logiin)
if "$NM" "$OUT" 2>/dev/null | grep -q 'rust_exported_session_'; then
    echo "[stcp] OK: rust_exported_session_* symbolit löytyivät $OUT:sta"
else
    echo "[stcp] WARN: rust_exported_session_* symboleita ei löytynyt $OUT:sta (tarkista FFI-exportit)" >&2
fi

# Kirjoitetaan Kbuildin .cmd-file (yksinkertainen versio, riittää useimpiin setuppeihin)
# Jos Kbuild ei tätä käytä, tämän voi jättää huomiotta.
mkdir -p "$(dirname "$CMD")"
cat >"$CMD" <<EOF
cmd_$OUT := ${LINK_CMD[*]}
EOF
