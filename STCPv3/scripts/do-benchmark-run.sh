#!/bin/bash
set -uo pipefail


ROUNDS=${1:-1}

for r in $(seq 1 "$ROUNDS"); do
    (
        tmp="/tmp/stcp-bench.round.$r"
        mkdir -p "$tmp"
        cd "$tmp"

        echo "=== ROUND $r START $(date --iso-8601=seconds) ==="

        bash ~/SDK/version-to-use/scripts/netconsole/enable-netconsole.sh

        rc=0
        #echo 0 | sudo tee /sys/module/stcp/parameters/verbose_debug
        #STCP_TEST_VERBOSE_DEBUG=0 bash ~/do-full-run.sh || rc=$?
        bash ~/do-full-run.sh || rc=$?
	#echo -n "Debug verbose: "
        #cat /sys/module/stcp/parameters/verbose_debug

        echo "=== do-full-run.sh rc=$rc ==="
        echo "=== COLLECTING POSTMORTEM ==="

        mortem_rc=0
        bash ~/SDK/v3/scripts/stcp-postmortem.sh || mortem_rc=$?

        echo "=== postmortem rc=$mortem_rc ==="

        shopt -s nullglob
        zips=(*.zip)
        if ((${#zips[@]})); then
            cp -v "${zips[@]}" ~/
        else
            echo "WARNING: no postmortem ZIP found in $tmp"
        fi

        echo "=== ROUND $r END $(date --iso-8601=seconds) ==="

        exit "$rc"
    ) |& ts "[Benchmark round $r / $ROUNDS :: %Y-%m-%d %H:%M:%S] "

    rc=${PIPESTATUS[0]}

    if (( rc != 0 )); then
        echo "[ Benchmark ] round $r FAILED rc=$rc"
        exit "$rc"
    fi
done
