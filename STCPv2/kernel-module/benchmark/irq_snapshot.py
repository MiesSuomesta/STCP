#!/usr/bin/env python3
"""Capture Raspberry/Linux IRQ, softirq and CPU-jiffy counters as JSON."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path


NETWORK_PATTERN_DEFAULT = r"(eth|enp|eno|end|bcmgenet|genet|lan|wlan|wifi|brcm|dwc|fec|gmac|eqos)"


def parse_interrupts(path: Path, pattern: re.Pattern[str]) -> dict[str, object]:
    total = 0
    network_total = 0
    network_lines: dict[str, int] = {}

    try:
        lines = path.read_text().splitlines()
    except OSError:
        return {"total": 0, "network_total": 0, "network_lines": {}}

    for line in lines[1:]:
        if ":" not in line:
            continue
        left, right = line.split(":", 1)
        tokens = right.split()
        counts: list[int] = []
        for token in tokens:
            if token.isdigit():
                counts.append(int(token))
            else:
                break
        if not counts:
            continue
        value = sum(counts)
        total += value
        descriptor = " ".join(tokens[len(counts):])
        if pattern.search(descriptor):
            network_total += value
            network_lines[f"{left.strip()} {descriptor}".strip()] = value

    return {
        "total": total,
        "network_total": network_total,
        "network_lines": network_lines,
    }


def parse_softirqs(path: Path) -> dict[str, int]:
    result: dict[str, int] = {}
    try:
        lines = path.read_text().splitlines()
    except OSError:
        return result

    for line in lines[1:]:
        if ":" not in line:
            continue
        name, values = line.split(":", 1)
        result[name.strip()] = sum(
            int(token) for token in values.split() if token.isdigit()
        )
    return result


def parse_cpu_stat(path: Path) -> dict[str, int]:
    try:
        line = next(
            line for line in path.read_text().splitlines()
            if line.startswith("cpu ")
        )
    except (OSError, StopIteration):
        return {}

    names = [
        "user", "nice", "system", "idle", "iowait",
        "irq", "softirq", "steal", "guest", "guest_nice",
    ]
    values = [int(value) for value in line.split()[1:]]
    return dict(zip(names, values))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "--network-pattern",
        default=NETWORK_PATTERN_DEFAULT,
        help="Regex used to identify network-related /proc/interrupts lines",
    )
    parser.add_argument("--output")
    args = parser.parse_args()

    pattern = re.compile(args.network_pattern, re.IGNORECASE)
    output = {
        "interrupts": parse_interrupts(Path("/proc/interrupts"), pattern),
        "softirqs": parse_softirqs(Path("/proc/softirqs")),
        "cpu_jiffies": parse_cpu_stat(Path("/proc/stat")),
    }

    encoded = json.dumps(output, sort_keys=True)
    if args.output:
        Path(args.output).write_text(encoded + "\n")
    print(encoded)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
