#!/usr/bin/env bash
set -euo pipefail
if (($# != 1)); then echo "usage: $0 RUN_DIR" >&2; exit 2; fi
RUN_DIR=$(realpath "$1")
OUT="$RUN_DIR/environment"
mkdir -p "$OUT"

capture() {
  name=$1; shift
  { "$@" || true; } >"$OUT/$name.txt" 2>&1
}

capture uname uname -a
capture cpu lscpu
capture memory free -b
capture network ip -details -statistics link
capture routes ip route show table all
capture sysctl sysctl net.core net.ipv4.tcp_congestion_control net.ipv4.tcp_rmem net.ipv4.tcp_wmem
capture openssl openssl version -a
capture python python3 --version
capture rustc rustc -vV
capture cargo cargo -V
capture stcp-module modinfo stcp
capture stcp-parameters sh -c 'for f in /sys/module/stcp/parameters/*; do test -e "$f" && printf "%s=" "${f##*/}" && cat "$f"; done'
capture temperature sh -c 'for f in /sys/class/thermal/thermal_zone*/temp; do test -r "$f" && printf "%s " "$f" && cat "$f"; done'
capture clocksource sh -c 'cat /sys/devices/system/clocksource/clocksource0/current_clocksource'

if [[ -n ${TARGET_SSH:-} ]]; then
  capture target "${TARGET_SSH_COMMAND:-ssh}" "$TARGET_SSH" \
    'uname -a; lscpu; free -b; ip -details -statistics link; openssl version -a 2>/dev/null || true; modinfo stcp 2>/dev/null || true; for f in /sys/module/stcp/parameters/*; do test -e "$f" && printf "%s=" "${f##*/}" && cat "$f"; done'
fi
