#!/usr/bin/env bash
set -Eeuo pipefail
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"

if [[ -x "$HERE/.venv/bin/python" ]]; then
    PY="$HERE/.venv/bin/python"
elif [[ -n "${VIRTUAL_ENV:-}" && -x "$VIRTUAL_ENV/bin/python" ]]; then
    PY="$VIRTUAL_ENV/bin/python"
else
    PY="$(command -v python3)"
fi

exec "$PY" "$HERE/run_benchmark.py" "$@"
