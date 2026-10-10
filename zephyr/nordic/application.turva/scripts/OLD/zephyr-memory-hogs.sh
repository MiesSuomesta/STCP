#!/usr/bin/env bash
set -euo pipefail

TOP="${TOP:-50}"
ELF="${1:-build-v2-clean/application/zephyr/zephyr.elf}"

[[ -f "$ELF" ]] || { echo "[ERROR] ELF not found: $ELF" >&2; echo "Usage: $0 [path/to/zephyr.elf]" >&2; exit 1; }
ELF="$(readlink -f "$ELF")"
MAP="${ELF%.elf}.map"

find_tool() {
    local n="$1" c
    if [[ -n "${CROSS_COMPILE:-}" ]] && command -v "${CROSS_COMPILE}${n}" >/dev/null 2>&1; then command -v "${CROSS_COMPILE}${n}"; return; fi
    for c in "${ZEPHYR_SDK_INSTALL_DIR:-}/arm-zephyr-eabi/bin/arm-zephyr-eabi-${n}" "$HOME/zephyr-sdk-0.16.8/arm-zephyr-eabi/bin/arm-zephyr-eabi-${n}"; do
        [[ -x "$c" ]] && { echo "$c"; return; }
    done
    command -v "arm-zephyr-eabi-${n}" 2>/dev/null || command -v "$n" 2>/dev/null || return 1
}

NM="$(find_tool nm)" || { echo '[ERROR] nm not found' >&2; exit 1; }
SIZE="$(find_tool size)" || { echo '[ERROR] size not found' >&2; exit 1; }
OBJDUMP="$(find_tool objdump)" || { echo '[ERROR] objdump not found' >&2; exit 1; }
hr(){ printf '%*s\n' 78 '' | tr ' ' '='; }
title(){ echo; hr; echo "$1"; hr; }

echo "Zephyr memory hog report"
echo "ELF : $ELF"
echo "TOP : $TOP"
[[ -f "$MAP" ]] && echo "MAP : $MAP"

title 'OVERALL SIZE'
"$SIZE" "$ELF"

title 'SECTIONS — largest first'
"$SIZE" -A "$ELF" | awk 'NR>2 && $2 ~ /^[0-9]+$/ {printf "%12d  %-28s  %s\n",$2,$1,$3}' | sort -nr | head -n "$TOP"

title 'BIGGEST RAM SYMBOLS — BSS + DATA'
"$NM" -S --size-sort --radix=d "$ELF" | awk '$3 ~ /^[BbDd]$/ {printf "%12d  %s  %s\n",$2,$3,$4}' | sort -nr | head -n "$TOP"

title 'STACK / THREAD / WORKQUEUE SUSPECTS'
"$NM" -S --size-sort --radix=d "$ELF" | awk 'BEGIN{IGNORECASE=1} $3 ~ /^[BbDd]$/ && $4 ~ /(stack|thread|workq|work_queue|rx|tx|bench)/ {printf "%12d  %s  %s\n",$2,$3,$4}' | sort -nr | head -n "$TOP"

title 'STCP RAM SYMBOLS'
"$NM" -S --size-sort --radix=d "$ELF" | awk 'BEGIN{IGNORECASE=1} $3 ~ /^[BbDd]$/ && $4 ~ /stcp/ {printf "%12d  %s  %s\n",$2,$3,$4}' | sort -nr | head -n "$TOP"

title 'BIGGEST FLASH/CODE SYMBOLS — TEXT + RODATA'
"$NM" -S --size-sort --radix=d "$ELF" | awk '$3 ~ /^[TtRr]$/ {printf "%12d  %s  %s\n",$2,$3,$4}' | sort -nr | head -n "$TOP"

title 'RAM-RELEVANT ELF SECTIONS'
"$OBJDUMP" -h "$ELF" | grep -Ei '\.bss|\.data|noinit|stack|heap' || true

if [[ -f "$MAP" ]]; then
    title 'MAP: STACK / HEAP / NOINIT / STCP HITS'
    grep -Ei 'stack|heap|noinit|stcp' "$MAP" | head -n 200 || true
fi

title 'QUICK SUMMARY'
RAM_TOTAL="$("$NM" -S --size-sort --radix=d "$ELF" | awk '$3 ~ /^[BbDd]$/ {s += $2} END {printf "%.0f",s+0}')"
STCP_RAM="$("$NM" -S --size-sort --radix=d "$ELF" | awk 'BEGIN{IGNORECASE=1} $3 ~ /^[BbDd]$/ && $4 ~ /stcp/ {s += $2} END {printf "%.0f",s+0}')"
printf 'Named BSS+DATA symbols total : %d bytes (%.2f KiB)\n' "$RAM_TOTAL" "$(awk -v n="$RAM_TOTAL" 'BEGIN{printf "%.2f",n/1024}')"
printf 'Named STCP RAM symbols       : %d bytes (%.2f KiB)\n' "$STCP_RAM" "$(awk -v n="$STCP_RAM" 'BEGIN{printf "%.2f",n/1024}')"
echo
echo "Tip: TOP=100 $0 \"$ELF\""
