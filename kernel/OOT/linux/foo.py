#!/usr/bin/env python3
import re
import sys
from pathlib import Path

# Match: if ( ... ) {
# Exclude: if let ( ... )
IF_RE = re.compile(
    r'''
    (^|\s)              # start or whitespace
    if                  # keyword
    \s*                 # optional space
    \(                  # opening paren
    ([^()]+)            # condition (no nested parens)
    \)                  # closing paren
    \s*                 # optional space
    \{                  # opening brace
    ''',
    re.VERBOSE
)

def fix_line(line: str) -> str:
    # Skip comments
    if line.lstrip().startswith("//"):
        return line

    # Skip if-let
    if "if let" in line:
        return line

    def repl(m):
        prefix = m.group(1)
        cond = m.group(2).strip()
        return f"{prefix}if {cond} {{"

    return IF_RE.sub(repl, line)

def process_file(path: Path):
    original = path.read_text()
    lines = original.splitlines(keepends=True)

    new_lines = [fix_line(line) for line in lines]
    new = "".join(new_lines)

    if new != original:
        path.write_text(new)
        print(f"[FIXED] {path}")

def main():
    if len(sys.argv) < 2:
        print("Usage: fix_rust_if_parens.py <file|dir> [...]")
        sys.exit(1)

    for arg in sys.argv[1:]:
        p = Path(arg)
        if p.is_file() and p.suffix == ".rs":
            process_file(p)
        elif p.is_dir():
            for f in p.rglob("*.rs"):
                process_file(f)

if __name__ == "__main__":
    main()
