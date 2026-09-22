#!/usr/bin/env bash
set -euo pipefail

MODULE="${MODULE:-stcp}"
PROC_USERS="${PROC_USERS:-/proc/stcp/users}"
TERM_WAIT="${TERM_WAIT:-5}"
KILL_WAIT="${KILL_WAIT:-3}"
SUDO="${SUDO:-sudo}"

module_loaded() {
    grep -q "^${MODULE} " /proc/modules
}

user_pids() {
    [[ -r "$PROC_USERS" ]] || return 0
    awk 'NR > 1 && $1 ~ /^[0-9]+$/ && $1 != 1 { print $1 }' "$PROC_USERS" | sort -nu
}

wait_users_gone() {
    local seconds="$1" i
    for ((i = 0; i < seconds * 10; i++)); do
        [[ -z "$(user_pids)" ]] && return 0
        sleep 0.1
    done
    return 1
}

if ! module_loaded; then
    echo "STCP module is not loaded."
    exit 0
fi

mapfile -t pids < <(user_pids)
if ((${#pids[@]})); then
    echo "Stopping STCP users gracefully: ${pids[*]}"
    "$SUDO" kill -TERM "${pids[@]}" 2>/dev/null || true

    if ! wait_users_gone "$TERM_WAIT"; then
        mapfile -t pids < <(user_pids)
        if ((${#pids[@]})); then
            echo "Forcing remaining STCP users to exit: ${pids[*]}"
            "$SUDO" kill -KILL "${pids[@]}" 2>/dev/null || true
            wait_users_gone "$KILL_WAIT" || true
        fi
    fi
fi

mapfile -t pids < <(user_pids)
if ((${#pids[@]})); then
    echo "ERROR: STCP sockets are still owned by PID(s): ${pids[*]}" >&2
    exit 1
fi

# modprobe -r handles module names correctly and refuses unsafe removal.
echo "Unloading ${MODULE}..."
"$SUDO" modprobe -r "$MODULE"
echo "${MODULE} unloaded cleanly."
