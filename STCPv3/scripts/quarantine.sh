#!/usr/bin/env bash
set -euo pipefail

REMOVE_HISTORY=0
if [[ "${1:-}" == "--remove" ]]; then
    REMOVE_HISTORY=1
    shift
fi

MAX_MIB="${1:-16}"
LIMIT="+${MAX_MIB}M"
ROOT="$(git rev-parse --show-toplevel)"
STAMP="$(date +%Y%m%d-%H%M%S)"
QUARANTINE="/srv/stcp-project/tmp/git-bigfiles-${STAMP}"

echo "Git root:   $ROOT"
echo "Quarantine: $QUARANTINE"
echo "Limit:      $LIMIT"
echo

mkdir -p "$QUARANTINE"

cd "$ROOT"

count=0
bytes=0

while IFS= read -r -d '' file; do
    rel="${file#./}"
    size="$(stat -c '%s' "$file")"

    printf '%10.1f MiB  %s\n' \
        "$(awk "BEGIN {print $size/1048576}")" \
        "$rel"

    mkdir -p "$QUARANTINE/$(dirname "$rel")"

    mv -- "$file" "$QUARANTINE/$rel"

    printf '%s\t%s\n' "$size" "$rel" >> "$QUARANTINE/MANIFEST.tsv"
    printf '%s\n' "$rel" >> "$QUARANTINE/FILTER_PATHS.txt"

    ((count+=1))
    ((bytes+=size))
done < <(
    find . \
        -path './.git' -prune -o \
        -type f -size "$LIMIT" -print0
)

echo
echo "Moved: $count files"
awk "BEGIN {printf \"Total: %.2f GiB\\n\", $bytes/1073741824}"
echo "Quarantine: $QUARANTINE"
echo
echo "Git status:"
git status --short

echo
if (( REMOVE_HISTORY )); then
    if (( count == 0 )); then
        echo "No files moved; Git history was not rewritten."
    else
        command -v git-filter-repo >/dev/null 2>&1 || {
            echo "ERROR: git-filter-repo not found." >&2
            exit 1
        }

        sort -u -o "$QUARANTINE/FILTER_PATHS.txt" "$QUARANTINE/FILTER_PATHS.txt"

        echo "Rewriting Git history; removing quarantined paths..."
        git filter-repo --force --invert-paths --paths-from-file "$QUARANTINE/FILTER_PATHS.txt"

        echo
        echo "History rewrite complete."
        echo "NOTE: git filter-repo may remove the origin remote; check: git remote -v"
    fi
else
    echo "NOTE:"
    echo "Files were moved from the working tree only."
    echo "Existing large blobs in Git history were NOT removed."
    echo "Use --remove to also remove quarantined paths from Git history."
fi
