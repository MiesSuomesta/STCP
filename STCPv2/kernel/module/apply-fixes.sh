#!/usr/bin/env bash
set -euo pipefail

ROOT="${1:-.}"
HEADER="$ROOT/include/stcp_socket.h"

if [[ ! -f "$HEADER" ]]; then
    echo "Virhe: $HEADER ei löydy" >&2
    exit 1
fi

if grep -Eq 'struct[[:space:]]+stcp_carrier[[:space:]]*\*[[:space:]]*carrier[[:space:]]*;' "$HEADER"; then
    echo "carrier-kenttä on jo $HEADER:ssa"
    exit 0
fi

python3 - "$HEADER" <<'PY'
from pathlib import Path
import sys

path = Path(sys.argv[1])
text = path.read_text()
needle = "\tvoid *rust_ctx;"
if needle not in text:
    needle = "    void *rust_ctx;"
if needle not in text:
    raise SystemExit("Virhe: rust_ctx-kenttää ei löytynyt struct stcp_sock -rakenteesta")
indent = needle[:len(needle) - len(needle.lstrip())]
replacement = needle + "\n" + indent + "struct stcp_carrier *carrier;"
text = text.replace(needle, replacement, 1)
path.write_text(text)
PY

echo "Lisätty struct stcp_carrier *carrier; tiedostoon $HEADER"
