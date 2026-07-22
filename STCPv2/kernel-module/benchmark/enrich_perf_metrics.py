#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import math
import re
from pathlib import Path
from typing import Any

EVENT_ALIASES = {
    "task-clock": "task_clock_ms",
    "context-switches": "context_switches",
    "cpu-migrations": "cpu_migrations",
    "page-faults": "page_faults",
    "cycles": "cycles",
    "instructions": "instructions",
    "branches": "branches",
    "branch-misses": "branch_misses",
    "cache-references": "cache_references",
    "cache-misses": "cache_misses",
}

def parse_number(value: str) -> float | None:
    value = value.strip().replace(" ", "")
    if not value or value.startswith("<"):
        return None
    value = value.replace(",", ".")
    try:
        number = float(value)
    except ValueError:
        return None
    return number if math.isfinite(number) else None

def normalize_event(value: str) -> str:
    return re.sub(r":[ukhp]+$", "", value.strip())

def parse_perf(path: Path) -> tuple[dict[str, float | None], list[str]]:
    metrics = {alias: None for alias in EVENT_ALIASES.values()}
    warnings: list[str] = []
    if not path.exists():
        return metrics, [f"perf output missing: {path}"]

    for raw in path.read_text(errors="replace").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        fields = [field.strip() for field in line.split(";")]
        if len(fields) < 3:
            continue
        value_text, _, event_text = fields[:3]
        alias = EVENT_ALIASES.get(normalize_event(event_text))
        if alias is None:
            continue
        value = parse_number(value_text)
        metrics[alias] = value
        if value is None:
            warnings.append(f"{event_text}: {value_text or 'not counted'}")
    return metrics, warnings

def ratio(value: float | None, denominator: float) -> float | None:
    if value is None or denominator <= 0:
        return None
    return value / denominator

def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--result", required=True)
    parser.add_argument("--perf", required=True)
    args = parser.parse_args()

    result_path = Path(args.result)
    result: dict[str, Any] = json.loads(result_path.read_text())
    metrics, warnings = parse_perf(Path(args.perf))

    operations = float(result.get("operations", 0) or 0)
    elapsed = float(result.get("elapsed_s", 0) or 0)
    total_mib = (
        float(result.get("tx_mib_s", 0) or 0)
        + float(result.get("rx_mib_s", 0) or 0)
    ) * elapsed

    cycles = metrics["cycles"]
    instructions = metrics["instructions"]
    branches = metrics["branches"]
    branch_misses = metrics["branch_misses"]
    cache_refs = metrics["cache_references"]
    cache_misses = metrics["cache_misses"]

    result.update({
        "server_perf_task_clock_ms": metrics["task_clock_ms"],
        "server_perf_context_switches": metrics["context_switches"],
        "server_perf_cpu_migrations": metrics["cpu_migrations"],
        "server_perf_page_faults": metrics["page_faults"],
        "server_perf_cycles": cycles,
        "server_perf_instructions": instructions,
        "server_perf_branches": branches,
        "server_perf_branch_misses": branch_misses,
        "server_perf_cache_references": cache_refs,
        "server_perf_cache_misses": cache_misses,
        "server_perf_cycles_per_op": ratio(cycles, operations),
        "server_perf_instructions_per_op": ratio(instructions, operations),
        "server_perf_context_switches_per_1k_ops":
            ratio(metrics["context_switches"], operations / 1000.0),
        "server_perf_cpu_migrations_per_1k_ops":
            ratio(metrics["cpu_migrations"], operations / 1000.0),
        "server_perf_page_faults_per_1k_ops":
            ratio(metrics["page_faults"], operations / 1000.0),
        "server_perf_cycles_per_mib": ratio(cycles, total_mib),
        "server_perf_instructions_per_mib": ratio(instructions, total_mib),
        "server_perf_task_clock_ms_per_1k_ops":
            ratio(metrics["task_clock_ms"], operations / 1000.0),
        "server_perf_ipc": (
            instructions / cycles
            if instructions is not None and cycles not in (None, 0)
            else None
        ),
        "server_perf_branch_miss_percent": (
            branch_misses / branches * 100.0
            if branch_misses is not None and branches not in (None, 0)
            else None
        ),
        "server_perf_cache_miss_percent": (
            cache_misses / cache_refs * 100.0
            if cache_misses is not None and cache_refs not in (None, 0)
            else None
        ),
        "server_perf_measurement": {
            "source": "perf stat -a",
            "scope": "Raspberry Pi server, system-wide",
            "warnings": warnings,
            "warning": "Processing-efficiency proxy only; not direct electrical power.",
        },
    })

    result_path.write_text(json.dumps(result, indent=2) + "\n")
    print(json.dumps({
        "result": str(result_path),
        "cycles_per_op": result["server_perf_cycles_per_op"],
        "instructions_per_op": result["server_perf_instructions_per_op"],
        "context_switches_per_1k_ops":
            result["server_perf_context_switches_per_1k_ops"],
        "warnings": warnings,
    }))
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
