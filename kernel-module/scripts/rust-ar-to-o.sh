#!/usr/bin/env bash
set -euo pipefail

# Käyttö: rust_ar_to_o.sh <input.a> <output.o> [cmdfile]
# - Luo .o ld -r -linkillä (bfd)
# - Kaivaa kaikki stcp_rust_* -symbolit ja pakottaa ne määritellyiksi
# - Kirjoittaa Kbuild-yhteensopivan .cmd -tiedoston (kolmas argumentti
#   tai automaattisesti .$OUT_BASENAME.cmd samassa hakemistossa)

if [ $# -lt 2 ] || [ $# -gt 3 ]; then
  echo "Usage: $0 <input.a> <output.o> [cmdfile]"
  exit 1
fi

IN="$1"
OUT="$2"
CMD_FILE="${3:-}"

if [[ ! -f "$IN" ]]; then
  echo "Error: input archive not found: $IN" >&2
  exit 1
fi

# Oletus .cmd-polku: <dir>/.<basename>.cmd  → esim. build/.stcp_rust_hooks.o.cmd
if [[ -z "${CMD_FILE}" ]]; then
  out_dir="$(dirname -- "$OUT")"
  out_base="$(basename -- "$OUT")"   # esim. stcp_rust_hooks.o
  CMD_FILE="${out_dir}/.${out_base}.cmd"
fi

NM_BIN="${NM:-nm}"
LD_BIN="${LD:-ld.bfd}"

echo "[stcp] Input:  $IN"
echo "[stcp] Output: $OUT"
echo "[stcp] .cmd:   $CMD_FILE"
echo "[stcp] nm:     ${NM_BIN}"
echo "[stcp] ld:     ${LD_BIN}"

# Kaiva julkiset stcp_rust_* -symbolit (pakotetaan ulos)
UNDEF_OPTS="$(
  "${NM_BIN}" -g --defined-only "$IN" \
    | awk '/[[:space:]]stcp_rust_/ { printf "--undefined=%s ", $3 }'
)"

echo "[stcp] --undefined opts: ${UNDEF_OPTS:-<none>}"

# Varsinainen linkkauskomento (kirjoitetaan myös .cmd:iin)
LD_CMD=( "${LD_BIN}" -r -m elf_x86_64 ${UNDEF_OPTS} -o "$OUT" "$IN" )

echo "[stcp] Linking → ${LD_CMD[*]}"
"${LD_CMD[@]}"

# Kirjoita Kbuild-tyylinen .cmd, jotta Make seuraa komentorivin muutoksia.
# Muoto riittää Kbuildille: cmd_<target> := <exact command line>
mkdir -p "$(dirname -- "$CMD_FILE")"

# Yritetään käyttää target-nimeä ilman välilyöntejä muuttujassa.
target_var="cmd_$(echo "$OUT" | sed 's/[^A-Za-z0-9_]/_/g')"

{
  printf '%s := ' "$target_var"
  # Yhden rivin täsmäkomento; quoteataan polut.
  printf '%q ' "${LD_CMD[@]}"
  printf '\n'
  # Lisätään vähän metatietoa talteen (ei pakollisia Kbuildille)
  echo "# in_archive := $IN"
  echo "# out_object := $OUT"
  echo "# undef_opts := ${UNDEF_OPTS}"
} > "$CMD_FILE"

echo "[stcp] Done: $OUT"
