#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import os
import re
import subprocess
import time
from pathlib import Path
from typing import Any

CASE_RE = re.compile(
    r"^(stcp-tcp|stcp-udp|tcp|tls|udp)-c(\d+)-p(\d+)-q(\d+)\.json$"
)


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
        return value if isinstance(value, dict) else {}
    except (OSError, json.JSONDecodeError):
        return {}


def passed(result: dict[str, Any]) -> bool:
    try:
        return (
            int(result.get("errors", 0)) == 0
            and int(result.get("operations", 0)) > 0
            and len(result.get("error_details") or []) == 0
        )
    except (TypeError, ValueError):
        return False


def format_payload(value: int) -> str:
    if value % (1024 * 1024) == 0:
        return f"{value // (1024 * 1024)} MiB"
    if value % 1024 == 0:
        return f"{value // 1024} KiB"
    return f"{value} B"


def notify(command: str, title: str, body: str, tuple_name: str) -> None:
    try:
        subprocess.run(
            [command, "-m" , "-a", f"STCP Benchmark // {tuple_name}", title, body],
            check=False,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
    except OSError:
        pass


def scan(result_dir: Path, kinds: list[str]) -> dict[tuple[int, int, int], dict[str, dict[str, Any]]]:
    tuples: dict[tuple[int, int, int], dict[str, dict[str, Any]]] = {}

    for carrier in ("tcp", "udp"):
        directory = result_dir / carrier
        if not directory.is_dir():
            continue

        for path in directory.glob("*.json"):
            match = CASE_RE.match(path.name)
            if not match:
                continue

            kind, clients, payload, pipeline = match.groups()
            if kind not in kinds:
                continue

            result = load_json(path)
            if not passed(result):
                continue

            key = (int(clients), int(payload), int(pipeline))
            tuples.setdefault(key, {})[kind] = result

    return tuples


def build_message(
    key: tuple[int, int, int],
    results: dict[str, dict[str, Any]],
    kinds: list[str],
) -> tuple[str, str]:
    clients, payload, pipeline = key
    title = f"STCP tuple ready: c{clients} p{format_payload(payload)} q{pipeline}"

    lines = []
    for kind in kinds:
        result = results[kind]
        combined = result.get("combined_mib_s")
        rtt = result.get("rtt_p50_ms")
        cpu = result.get("client_cpu_percent")

        def num(value: Any, digits: int = 2) -> str:
            try:
                return f"{float(value):.{digits}f}"
            except (TypeError, ValueError):
                return "-"

        lines.append(
            f"{kind}: {num(combined)} MiB/s, "
            f"p50 {num(rtt)} ms, CPU {num(cpu, 1)}%"
        )

    return title, "\n".join(lines)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("result_dir")
    parser.add_argument(
        "--kinds",
        default=os.environ.get("TUPLE_KINDS", "stcp-tcp,tls,tcp,stcp-udp"),
        help="Comma-separated required result kinds",
    )
    parser.add_argument(
        "--poll-seconds",
        type=float,
        default=float(os.environ.get("POLL_SECONDS", "2")),
    )
    parser.add_argument(
        "--pncnote",
        default=os.environ.get("PNCNOTE", "pncnote"),
    )
    parser.add_argument(
        "--once",
        action="store_true",
        help="Scan once and exit",
    )
    args = parser.parse_args()

    result_dir = Path(args.result_dir).resolve()
    if not result_dir.is_dir():
        raise SystemExit(f"Result directory not found: {result_dir}")

    kinds = [item.strip() for item in args.kinds.split(",") if item.strip()]
    if len(kinds) < 2:
        raise SystemExit("At least two kinds are required")

    state_file = result_dir / ".tuple-watcher-state.json"
    notified: set[str] = set()

    if state_file.is_file():
        try:
            state = json.loads(state_file.read_text(encoding="utf-8"))
            notified.update(state.get("notified", []))
        except (OSError, json.JSONDecodeError):
            pass

    notify(
        args.pncnote,
        "STCP tuple watcher started",
        f"{result_dir.name} | waiting for: {', '.join(kinds)}",
	"Starting...."
    )

    while True:
        tuples = scan(result_dir, kinds)
        changed = False

        for key, results in sorted(tuples.items(), reverse=True):
            if not all(kind in results for kind in kinds):
                continue

            tuple_str = f"c{key[0]}-p{key[1]}-q{key[2]}"
            tuple_id = f"c{key[0]}-p{key[1]}-q{key[2]}:" + ",".join(kinds)
            if tuple_id in notified:
                continue

            title, body = build_message(key, results, kinds)
            print(title)
            print(body)
            print()

            notify(args.pncnote, title, body, tuple_str)
            notified.add(tuple_id)
            changed = True

        if changed:
            temporary = state_file.with_suffix(".tmp")
            temporary.write_text(
                json.dumps(
                    {"notified": sorted(notified)},
                    indent=2,
                    ensure_ascii=False,
                )
                + "\n",
                encoding="utf-8",
            )
            os.replace(temporary, state_file)

        if args.once:
            break

        time.sleep(max(0.5, args.poll_seconds))

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
