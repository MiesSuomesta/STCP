#!/usr/bin/env python3
"""Generate pipeline-summary.json from an existing benchmark result directory.

No benchmark cases are executed.

Usage:
    python3 generate-summary-from-results.py RESULT_DIR [--mode both]

The script reads:
- primary case JSON files below tcp/ and udp/
- run-summary.tsv when available
- cases.tsv when available
- file modification times for run start/end estimation
"""

from __future__ import annotations

import argparse
import json
import os
import re
from datetime import datetime, timezone, timedelta
from pathlib import Path
from typing import Any

CASE_RE = re.compile(
    r"^(tcp|tls|stcp-tcp|udp|stcp-udp)-c(\d+)-p(\d+)-q(\d+)\.json$"
)


def iso(epoch: float | None) -> str | None:
    if epoch is None:
        return None
    return datetime.fromtimestamp(epoch, tz=timezone.utc).astimezone().isoformat()


def load_json(path: Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
        return value if isinstance(value, dict) else {}
    except (OSError, json.JSONDecodeError):
        return {}


def infer_configured_duration(result: dict[str, Any]) -> float | None:
    elapsed = result.get("elapsed_s")
    try:
        value = float(elapsed)
    except (TypeError, ValueError):
        return None

    common = (1, 5, 15, 20, 30, 60, 120, 150, 180, 300)
    return float(min(common, key=lambda n: abs(value - n)))


def discover_cases(result_dir: Path) -> list[dict[str, Any]]:
    cases: list[dict[str, Any]] = []

    for carrier in ("tcp", "udp"):
        directory = result_dir / carrier
        if not directory.is_dir():
            continue

        for path in sorted(directory.glob("*.json")):
            match = CASE_RE.match(path.name)
            if not match:
                continue

            kind, clients, payload, pipeline = match.groups()
            result = load_json(path)
            errors = result.get("errors", 0)
            operations = result.get("operations", 0)
            details = result.get("error_details") or []

            try:
                passed = int(errors) == 0 and int(operations) > 0 and len(details) == 0
            except (TypeError, ValueError):
                passed = False

            finished_epoch = path.stat().st_mtime
            benchmark_elapsed = result.get("elapsed_s")
            try:
                benchmark_elapsed = float(benchmark_elapsed)
            except (TypeError, ValueError):
                benchmark_elapsed = None

            started_epoch = (
                finished_epoch - benchmark_elapsed
                if benchmark_elapsed is not None
                else finished_epoch
            )

            cases.append(
                {
                    "kind": kind,
                    "carrier": carrier,
                    "clients": int(clients),
                    "payload_bytes": int(payload),
                    "pipeline": int(pipeline),
                    "configured_duration_s": infer_configured_duration(result),
                    "status": "PASS" if passed else "FAIL",
                    "started_at": iso(started_epoch),
                    "finished_at": iso(finished_epoch),
                    "wall_duration_s": benchmark_elapsed,
                    "benchmark_elapsed_s": benchmark_elapsed,
                    "operations": result.get("operations"),
                    "errors": result.get("errors"),
                    "combined_mib_s": result.get("combined_mib_s"),
                    "operations_s": result.get("operations_s"),
                    "rtt_p50_ms": result.get("rtt_p50_ms"),
                    "rtt_p95_ms": result.get("rtt_p95_ms"),
                    "rtt_p99_ms": result.get("rtt_p99_ms"),
                    "result_file": str(path.relative_to(result_dir)),
                    "_started_epoch": started_epoch,
                    "_finished_epoch": finished_epoch,
                }
            )

    cases.sort(
        key=lambda c: (
            c["_finished_epoch"],
            c["carrier"],
            c["kind"],
            c["clients"],
            c["payload_bytes"],
            c["pipeline"],
        )
    )

    for index, case in enumerate(cases, start=1):
        case["index"] = index
        case.pop("_started_epoch", None)
        case.pop("_finished_epoch", None)

    return cases


def count_manifest_cases(result_dir: Path) -> int | None:
    path = result_dir / "cases.tsv"
    if not path.is_file():
        return None

    lines = [
        line for line in path.read_text(encoding="utf-8").splitlines()
        if line.strip()
    ]
    return max(0, len(lines) - 1)


def infer_mode(cases: list[dict[str, Any]]) -> str:
    carriers = {case["carrier"] for case in cases}
    if carriers == {"tcp", "udp"}:
        return "both"
    if carriers == {"tcp"}:
        return "tcp"
    if carriers == {"udp"}:
        return "udp"
    return "both"


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("result_dir")
    parser.add_argument("--mode", choices=("tcp", "udp", "both"))
    parser.add_argument("--output")
    args = parser.parse_args()

    result_dir = Path(args.result_dir).resolve()
    if not result_dir.is_dir():
        raise SystemExit(f"Result directory not found: {result_dir}")

    cases = discover_cases(result_dir)
    if not cases:
        raise SystemExit(f"No primary benchmark JSON files found in {result_dir}")

    started_epochs = []
    finished_epochs = []

    for case in cases:
        if case["started_at"]:
            started_epochs.append(datetime.fromisoformat(case["started_at"]).timestamp())
        if case["finished_at"]:
            finished_epochs.append(datetime.fromisoformat(case["finished_at"]).timestamp())

    started_epoch = min(started_epochs)
    finished_epoch = max(finished_epochs)
    total_wall = max(0, round(finished_epoch - started_epoch))

    expected = count_manifest_cases(result_dir) or len(cases)
    passed = sum(1 for case in cases if case["status"] == "PASS")
    failed = sum(1 for case in cases if case["status"] == "FAIL")
    completed = len(cases)

    configured_values = [
        case["configured_duration_s"]
        for case in cases
        if case["configured_duration_s"] is not None
    ]
    configured_duration = (
        max(set(configured_values), key=configured_values.count)
        if configured_values
        else None
    )

    status = (
        "complete"
        if completed == expected and failed == 0
        else "failed"
        if failed
        else "partial"
    )

    summary = {
        "schema_version": 1,
        "generated_from_existing_results": True,
        "run_id": result_dir.name,
        "mode": args.mode or infer_mode(cases),
        "status": status,
        "started_at": iso(started_epoch),
        "updated_at": datetime.now().astimezone().isoformat(),
        "finished_at": iso(finished_epoch),
        "total_wall_duration_s": total_wall,
        "total_wall_duration_human": str(timedelta(seconds=total_wall)),
        "configured_case_duration_s": configured_duration,
        "case_count_expected": expected,
        "case_count_completed": completed,
        "progress_percent": int(completed * 100 / expected) if expected else 0,
        "passed": passed,
        "failed": failed,
        "average_case_wall_duration_s": (
            sum((case["wall_duration_s"] or 0) for case in cases) / completed
        ),
        "result_directory": str(result_dir),
        "timing_accuracy": {
            "run_start_end": "estimated from primary result JSON modification times",
            "case_wall_duration": "uses benchmark elapsed_s; retries and restart overhead unavailable",
        },
        "cases": cases,
    }

    output = (
        Path(args.output).resolve()
        if args.output
        else result_dir / "pipeline-summary.json"
    )
    output.parent.mkdir(parents=True, exist_ok=True)

    temporary = output.with_suffix(output.suffix + ".tmp")
    temporary.write_text(
        json.dumps(summary, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    os.replace(temporary, output)

    print(output)
    print(
        f"cases={completed} passed={passed} failed={failed} "
        f"duration={summary['total_wall_duration_human']}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
