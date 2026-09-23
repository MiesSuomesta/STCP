#!/bin/bash
set -e

ROUNDS=1
SKIP_COMPILE=0

usage() {
    echo "Usage: $0 [ROUNDS] [--skip-compile]"
}

for arg in "$@"; do
    case "$arg" in
        --skip-compile) SKIP_COMPILE=1 ;;
        -h|--help) usage; exit 0 ;;
        ''|*[!0-9]*) echo "Unknown option: $arg" >&2; usage >&2; exit 2 ;;
        *) ROUNDS="$arg" ;;
    esac
done

FULL_RUN_ARGS=()
(( SKIP_COMPILE )) && FULL_RUN_ARGS+=(--skip-compile)

for r in $(seq 1 "$ROUNDS")
do
    (
        tmp=/tmp/stcp-bench.round.$r
        mkdir -p "$tmp"

        cd "$tmp"
        bash /srv/stcp-project/SDK/version-to-use/scripts/netconsole/enable-netconsole.sh
        bash /srv/stcp-project/STCP/version-to-use/do-full-run.sh "${FULL_RUN_ARGS[@]}"
        bash /srv/stcp-project/SDK/version-to-use/scripts/stcp-postmortem.sh
        cp -v ./*.zip /srv/stcp-project/
    ) |& ts "[Benchmark round $r / $ROUNDS :: %Y-%m-%d %H:%M:%S] "
done
