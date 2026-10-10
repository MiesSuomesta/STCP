#!/usr/bin/env bash
set -Eeuo pipefail
cd "$(dirname "$0")"
task_dir=$(mktemp -d)
trap 'rm -rf "$task_dir"' EXIT
cc -O2 -std=gnu11 -Wall -Wextra -Werror -pthread ../testing/test-server/ben2_server.c -o "$task_dir/server"
cc -O2 -std=gnu11 -Wall -Wextra -Werror -pthread -include server-socket-shim.h ../testing/test-server/ben2_server.c -o "$task_dir/server-sim"
for carrier in lte ethernet; do
    flags=(); expected=2
    if [[ "$carrier" == ethernet ]]; then flags=(-DCONFIG_STCP_TRANSPORT_ETHERNET=1); expected=1; fi
    cc -O2 -std=gnu11 -Wall -Wextra -Werror -pthread "${flags[@]}" -DHOST_SIMULATE_STCP=1 \
        -Ishim -I../src ../src/echo_benchmark.c main.c -o "$task_dir/client"
    python3 check-transports.py "$task_dir/client" "$task_dir/server" "$expected"
    cc -O2 -std=gnu11 -Wall -Wextra -Werror -pthread "${flags[@]}" -DHOST_SIMULATE_STCP=1 -DHOST_SIMULATE_STCP_STREAM=1 \
        -Ishim -I../src ../src/echo_benchmark.c main.c -o "$task_dir/client-stream"
    HOST_SERVER_SIMULATED=1 python3 check-transports.py "$task_dir/client-stream" "$task_dir/server-sim" "$expected"
done
cc -O2 -std=gnu11 -Wall -Wextra -Werror -pthread -DHOST_SIMULATE_STCP=1 -DHOST_SIMULATE_STCP_STREAM=1 -DHOST_READ_SLICE=7 \
    -Ishim -I../src ../src/echo_benchmark.c main.c -o "$task_dir/client-split"
HOST_SERVER_SIMULATED=1 HOST_SHORT_SUITE=1 python3 check-transports.py "$task_dir/client-split" "$task_dir/server-sim" 2
python3 check-udp-errors.py "$task_dir/server" "$task_dir/client"
