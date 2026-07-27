#!/usr/bin/env python3

from pathlib import Path

RESULTS = Path("benchmark/results")


def failed(tsv: Path) -> bool:
    if not tsv.exists():
        return False

    for line in tsv.read_text().splitlines():
        if line.strip():
            return True

    return False


def directory_ok(path: Path) -> bool:
    return (
        not failed(path / "FAILED-TCP-CASES.tsv")
        and
        not failed(path / "FAILED-UDP-CASES.tsv")
    )


def main():
    dirs = sorted(
        (
            p for p in RESULTS.iterdir()
            if p.is_dir() and p.name.startswith("full-")
        ),
        reverse=True
    )

    for d in dirs:
        if directory_ok(d):
            print(d.resolve())
            return

    print("No fully successful benchmark directories found.")
    raise SystemExit(1)


if __name__ == "__main__":
    main()
