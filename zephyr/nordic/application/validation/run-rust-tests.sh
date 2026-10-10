#!/usr/bin/env bash
set -Eeuo pipefail
cd "$(dirname "$0")"
task_dir=$(mktemp -d)
trap 'rm -rf "$task_dir"' EXIT
BEN2_RESULTS="$task_dir/unit.jsonl" cargo test --manifest-path ../testing/test-server/Cargo.toml
cargo build --release --manifest-path ../testing/test-server/Cargo.toml
server="$(pwd)/../testing/test-server/target/release/ben2-server"
for carrier in lte ethernet; do
    flags=(); expected=2
    if [[ "$carrier" == ethernet ]]; then flags=(-DCONFIG_STCP_TRANSPORT_ETHERNET=1); expected=1; fi
    cc -O2 -std=gnu11 -Wall -Wextra -Werror -pthread "${flags[@]}" -DHOST_SIMULATE_STCP=1 \
        -Ishim -I../src ../src/echo_benchmark.c main.c -o "$task_dir/client"
    # STCP application choices use test-only socket adapters here; server uses plain TCP/UDP.
    python3 check-transports.py "$task_dir/client" "$server" "$expected"
done
python3 check-udp-errors.py "$server" "$task_dir/client"
python3 check-progress.py "$server"
