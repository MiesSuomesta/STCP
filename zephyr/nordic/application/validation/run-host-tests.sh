#!/usr/bin/env bash
set -Eeuo pipefail
cd "$(dirname "$0")"
task_dir=$(mktemp -d)
trap 'rm -rf "$task_dir"' EXIT
for carrier in lte ethernet; do
    flags=()
    expected=2
    if [[ "$carrier" == ethernet ]]; then flags=(-DCONFIG_STCP_TRANSPORT_ETHERNET=1); expected=1; fi
    cc -std=gnu11 -Wall -Wextra -Werror -pthread "${flags[@]}" \
       -Ishim -I../src ../src/echo_benchmark.c main.c -o "$task_dir/bench"
    cc -std=gnu11 -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-variable \
       "${flags[@]}" -Ishim -I../src -fsyntax-only ../src/bench_shell.c
    python3 check.py "$task_dir/bench" "$expected"
done
