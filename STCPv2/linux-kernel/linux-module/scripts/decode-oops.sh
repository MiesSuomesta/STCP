#!/usr/bin/env bash
#
# decode-oops.sh
#
# Ratkaisee kernelin Oops/KASAN/panic-lokissa olevat osoitteet.
#
# Käyttö:
#   ./decode-oops.sh konsoli.log
#
# Tai stdinistä:
#   dmesg | ./decode-oops.sh -
#
set -Eeuo pipefail

# ============================================================
# Kiinteät polut
# ============================================================

KERNEL_SRC="/home/pomo/git/STCP/STCPv2/linux-kernel/rust/linux-next"
VMLINUX="/home/pomo/git/STCP/STCPv2/linux-kernel/rust/linux-next/vmlinux"

STCP_MODULE="/home/pomo/git/STCP/STCPv2/linux-kernel/linux-module/stcp.ko"

FADDR2LINE="${KERNEL_SRC}/scripts/faddr2line"
DECODE_STACKTRACE="${KERNEL_SRC}/scripts/decode_stacktrace.sh"

# LLVM/Binutils-työkalut.
ADDR2LINE="${ADDR2LINE:-addr2line}"
READELF="${READELF:-readelf}"
NM="${NM:-nm}"

usage() {
    cat <<EOF
Käyttö:
  $0 <oops.log|- >

Esimerkit:
  $0 konsoli.log
  sudo dmesg | $0 -
  $0 konsoli.log | tee decoded-oops.log

Kiinteät polut:
  Kernel:  $VMLINUX
  Lähteet: $KERNEL_SRC
  Moduuli: $STCP_MODULE
EOF
}

if [[ $# -ne 1 ]]; then
    usage >&2
    exit 1
fi

INPUT="$1"

die() {
    echo "Virhe: $*" >&2
    exit 1
}

warn() {
    echo "Varoitus: $*" >&2
}

# ============================================================
# Tarkistukset
# ============================================================

[[ -d "$KERNEL_SRC" ]] ||
    die "kernel-lähdepuuta ei löytynyt: $KERNEL_SRC"

[[ -f "$VMLINUX" ]] ||
    die "vmlinux-tiedostoa ei löytynyt: $VMLINUX"

[[ -r "$VMLINUX" ]] ||
    die "vmlinux ei ole luettavissa: $VMLINUX"

[[ -f "$FADDR2LINE" ]] ||
    die "faddr2line-työkalua ei löytynyt: $FADDR2LINE"

if [[ "$INPUT" != "-" && ! -f "$INPUT" ]]; then
    die "lokitiedostoa ei löytynyt: $INPUT"
fi

for tool in awk grep sed sort mktemp file "$ADDR2LINE" "$READELF" "$NM"; do
    command -v "$tool" >/dev/null 2>&1 ||
        die "tarvittava työkalu puuttuu: $tool"
done

TMPDIR="$(mktemp -d)"
trap 'rm -rf "$TMPDIR"' EXIT

LOG="${TMPDIR}/oops.log"

if [[ "$INPUT" == "-" ]]; then
    cat > "$LOG"
else
    cp -- "$INPUT" "$LOG"
fi

[[ -s "$LOG" ]] || die "loki on tyhjä"

# ============================================================
# Tiedot käytettävistä ELF-tiedostoista
# ============================================================

echo "============================================================"
echo "KÄYTETTÄVÄT SYMBOLITIEDOSTOT"
echo "============================================================"

echo
echo "Kernel:"
echo "  $VMLINUX"
file "$VMLINUX" | sed 's/^/  /'

VMLINUX_BUILD_ID="$(
    "$READELF" -n "$VMLINUX" 2>/dev/null |
    awk '/Build ID:/ { print $3; exit }'
)"

if [[ -n "$VMLINUX_BUILD_ID" ]]; then
    echo "  Build ID: $VMLINUX_BUILD_ID"
fi

if ! "$READELF" -S "$VMLINUX" 2>/dev/null |
    grep -qE '\.debug_info|\.debug_line'; then
    warn "vmlinux ei näytä sisältävän DWARF-debug-tietoja"
fi

echo
echo "STCP-moduuli:"

if [[ -f "$STCP_MODULE" ]]; then
    echo "  $STCP_MODULE"
    file "$STCP_MODULE" | sed 's/^/  /'

    if ! "$READELF" -S "$STCP_MODULE" 2>/dev/null |
        grep -qE '\.debug_info|\.debug_line'; then
        warn "STCP-moduuli ei näytä sisältävän DWARF-debug-tietoja"
    fi
else
    echo "  Ei löytynyt: $STCP_MODULE"
    echo "  Moduulisymbolien ratkaisu ohitetaan."
fi

# ============================================================
# Koko stack tracen automaattinen purku
# ============================================================

echo
echo "============================================================"
echo "KERNELIN DECODE_STACKTRACE"
echo "============================================================"
echo

if [[ -f "$DECODE_STACKTRACE" ]]; then
    if ! bash "$DECODE_STACKTRACE" "$VMLINUX" "$KERNEL_SRC" < "$LOG"; then
        warn "decode_stacktrace.sh palautti virheen"
    fi
else
    warn "decode_stacktrace.sh puuttuu: $DECODE_STACKTRACE"
fi

# ============================================================
# Symbolien poiminta
# ============================================================

KERNEL_SYMBOLS="${TMPDIR}/kernel-symbols.txt"
MODULE_SYMBOLS="${TMPDIR}/module-symbols.txt"
ALL_SYMBOLS="${TMPDIR}/all-symbols.txt"

grep -Eo \
    '[[:alnum:]_.$:@-]+\+0x[[:xdigit:]]+/0x[[:xdigit:]]+' \
    "$LOG" |
    sed -E 's/[),;:]$//' |
    sort -u > "$ALL_SYMBOLS" || true

: > "$KERNEL_SYMBOLS"
: > "$MODULE_SYMBOLS"

while IFS= read -r symbol; do
    [[ -n "$symbol" ]] || continue

    function_name="${symbol%%+*}"

    if [[ -f "$STCP_MODULE" ]] &&
       "$NM" -a "$STCP_MODULE" 2>/dev/null |
       awk '{print $NF}' |
       grep -Fxq "$function_name"; then
        echo "$symbol" >> "$MODULE_SYMBOLS"
    else
        echo "$symbol" >> "$KERNEL_SYMBOLS"
    fi
done < "$ALL_SYMBOLS"

sort -u -o "$KERNEL_SYMBOLS" "$KERNEL_SYMBOLS"
sort -u -o "$MODULE_SYMBOLS" "$MODULE_SYMBOLS"

# ============================================================
# Kernel-symbolit
# ============================================================

echo
echo "============================================================"
echo "KERNEL-SYMBOLIT"
echo "============================================================"

if [[ ! -s "$KERNEL_SYMBOLS" ]]; then
    echo
    echo "Kernel-symboleita ei löytynyt."
else
    while IFS= read -r symbol; do
        echo
        echo "------------------------------------------------------------"
        echo "$symbol"
        echo "------------------------------------------------------------"

        bash "$FADDR2LINE" "$VMLINUX" "$symbol" 2>&1 ||
            echo "Ei ratkennut kernelin vmlinux-tiedostolla."
    done < "$KERNEL_SYMBOLS"
fi

# ============================================================
# STCP-moduulin symbolit
# ============================================================

echo
echo "============================================================"
echo "STCP-MODUULIN SYMBOLIT"
echo "============================================================"

if [[ ! -f "$STCP_MODULE" ]]; then
    echo
    echo "STCP-moduulia ei löytynyt."
elif [[ ! -s "$MODULE_SYMBOLS" ]]; then
    echo
    echo "STCP-moduuliin kuuluvia symboleita ei tunnistettu."
else
    while IFS= read -r symbol; do
        echo
        echo "------------------------------------------------------------"
        echo "$symbol"
        echo "------------------------------------------------------------"

        bash "$FADDR2LINE" "$STCP_MODULE" "$symbol" 2>&1 ||
            echo "Ei ratkennut STCP-moduulin avulla."
    done < "$MODULE_SYMBOLS"
fi

# ============================================================
# Paljaat kernelosoitteet
# ============================================================

ADDRESSES="${TMPDIR}/addresses.txt"

grep -Eo \
    '0x[fF]{2}[[:xdigit:]]{6,16}|[fF]{4}[[:xdigit:]]{8,16}' \
    "$LOG" |
    sed -E 's/^0x//' |
    tr '[:upper:]' '[:lower:]' |
    sort -u > "$ADDRESSES" || true

echo
echo "============================================================"
echo "PALJAAT KERNEL-OSOITTEET"
echo "============================================================"

if [[ ! -s "$ADDRESSES" ]]; then
    echo
    echo "Paljaita kernelosoitteita ei löytynyt."
else
    while IFS= read -r address; do
        echo
        echo "------------------------------------------------------------"
        echo "0x${address}"
        echo "------------------------------------------------------------"

        "$ADDR2LINE" \
            --exe="$VMLINUX" \
            --functions \
            --demangle \
            --inlines \
            --pretty-print \
            "0x${address}" 2>&1 || true
    done < "$ADDRESSES"
fi

# ============================================================
# Loki riveittäin
# ============================================================

echo
echo "============================================================"
echo "LOKIRIVIT JA RATKAISTUT OSOITTEET"
echo "============================================================"

while IFS= read -r line; do
    echo
    echo "LOG: $line"

    while IFS= read -r symbol; do
        [[ -n "$symbol" ]] || continue

        function_name="${symbol%%+*}"
        elf="$VMLINUX"
        elf_name="kernel"

        if [[ -f "$STCP_MODULE" ]] &&
           "$NM" -a "$STCP_MODULE" 2>/dev/null |
           awk '{print $NF}' |
           grep -Fxq "$function_name"; then
            elf="$STCP_MODULE"
            elf_name="stcp.ko"
        fi

        echo "  SYMBOL [$elf_name]: $symbol"

        bash "$FADDR2LINE" "$elf" "$symbol" 2>/dev/null |
            sed 's/^/    /' || true

    done < <(
        grep -Eo \
            '[[:alnum:]_.$:@-]+\+0x[[:xdigit:]]+/0x[[:xdigit:]]+' \
            <<< "$line" |
        sort -u || true
    )

    while IFS= read -r address; do
        [[ -n "$address" ]] || continue

        address="${address#0x}"

        echo "  ADDRESS [kernel]: 0x${address}"

        "$ADDR2LINE" \
            --exe="$VMLINUX" \
            --functions \
            --demangle \
            --inlines \
            --pretty-print \
            "0x${address}" 2>/dev/null |
            sed 's/^/    /' || true

    done < <(
        grep -Eo \
            '0x[fF]{2}[[:xdigit:]]{6,16}|[fF]{4}[[:xdigit:]]{8,16}' \
            <<< "$line" |
        sort -u || true
    )

done < <(
    grep -E \
        'Call Trace:|RIP:|EIP:|PC is at|LR is at|[[:alnum:]_.$:@-]+\+0x[[:xdigit:]]+/0x[[:xdigit:]]+|0x[fF]{2}[[:xdigit:]]{6,16}|[fF]{4}[[:xdigit:]]{8,16}' \
        "$LOG" || true
)

echo
echo "============================================================"
echo "VALMIS"
echo "============================================================"
echo
echo "Kernel:"
echo "  $VMLINUX"
echo
echo "STCP-moduuli:"
echo "  $STCP_MODULE"
echo
