#!/usr/bin/env bash
set -euo pipefail

SR=$(cd $(dirname $0)/.. && pwd)

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
UNDEF_OPTS_RUST_GLUE="$(
  "${NM_BIN}" -g --defined-only "$IN" \
    | awk '/[[:space:]]stcp_rust__glue_/ { printf "--undefined=%s ", $3 }'
)"

UNDEF_OPTS_RUST_EXPORTS="$(
  "${NM_BIN}" -g --defined-only "$IN" \
    | awk '/[[:space:]]rust_exported_/ { printf "--undefined=%s ", $3 }'
)"

UNDEF_OPTS_STCP="$(
  "${NM_BIN}" -g --defined-only "$IN" \
    | awk '/[[:space:]]stcp_/ { printf "--undefined=%s ", $3 }'
)"

UNDEF_OPTS_STCP_MODULE="$(
  "${NM_BIN}" -g --defined-only "$IN" \
    | awk '/[[:space:]]stcp_module_rust_/ { printf "--undefined=%s ", $3 }'
)"

UNDEF_OPTS_STCP_EXPORTS="$(
  "${NM_BIN}" -g --defined-only "$IN" \
    | awk '/[[:space:]]stcp_exported_/ { printf "--undefined=%s ", $3 }'
)"

UNDEF_OPTS_STCP_PROTO_OPS="$(
  "${NM_BIN}" -g --defined-only "$IN" \
    | awk '/[[:space:]]stcp_proto_/ { printf "--undefined=%s ", $3 }'
)"

UNDEF_OPTS_STCP_STATE="$(
  "${NM_BIN}" -g --defined-only "$IN" \
    | awk '/[[:space:]]stcp_state_/ { printf "--undefined=%s ", $3 }'
)"


echo "[stcp] --undefined opts: ${UNDEF_OPTS_RUST_GLUE} ${UNDEF_OPTS_RUST_EXPORTS} ${UNDEF_OPTS_STCP} ${UNDEF_OPTS_STCP_MODULE} ${UNDEF_OPTS_STCP_EXPORTS} ${UNDEF_OPTS_STCP_STATE}"

# Varsinainen linkkauskomento (kirjoitetaan myös .cmd:iin)
LD_CMD=( "${LD_BIN}" --whole-archive -r -m elf_x86_64 \
	${UNDEF_OPTS_RUST_GLUE} ${UNDEF_OPTS_RUST_EXPORTS} ${UNDEF_OPTS_STCP} \
        ${UNDEF_OPTS_STCP_MODULE} ${UNDEF_OPTS_STCP_EXPORTS} \
        ${UNDEF_OPTS_STCP_PROTO_OPS} ${UNDEF_OPTS_STCP_STATE} \
	 -o "$OUT" "$IN" )

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
  echo "# undef_opts := ${UNDEF_OPTS_RUST_GLUE} ${UNDEF_OPTS_RUST_EXPORTS} ${UNDEF_OPTS_STCP} ${UNDEF_OPTS_STCP_MODULE} ${UNDEF_OPTS_STCP_EXPORTS} ${UNDEF_OPTS_STCP_PROTO_OPS}"
} > "$CMD_FILE"

echo "[stcp] Done: $OUT"
