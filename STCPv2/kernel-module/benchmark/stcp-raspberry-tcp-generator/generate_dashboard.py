#!/usr/bin/env python3
"""Generate a static STCP benchmark dashboard from benchmark result JSON files.

No third-party Python dependencies are required.
"""
from __future__ import annotations

import argparse
import csv
import datetime as dt
import hashlib
import json
import os
import re
import shutil
import statistics
import sys
from collections import Counter, defaultdict
from pathlib import Path
from typing import Any, Iterable

CASE_RE = re.compile(
    r"^(?P<prefix>tcp|tls|stcp-(?P<carrier>tcp|udp))-c(?P<clients>\d+)-p(?P<payload>\d+)-q(?P<pipeline>\d+)\.json$"
)
PROTOCOL_ORDER = {"tcp": 0, "stcp": 1, "tls": 2}
PROTOCOL_LABEL = {"tcp": "TCP", "tls": "TLS", "stcp": "STCP"}

METRICS = [
    "operations_s",
    "combined_mib_s",
    "rtt_p50_ms",
    "rtt_p95_ms",
    "rtt_p99_ms",
    "connect_mean_ms",
    "client_cpu_percent",
    "max_rss_kib",
    "server_cpu_busy_percent",
    "server_kernel_network_events_per_1k_ops",
    "server_perf_cycles_per_op",
    "server_perf_instructions_per_op",
    "server_perf_context_switches_per_1k_ops",
    "server_perf_ipc",
    "server_perf_cache_miss_percent",
]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", type=Path, help="Benchmark result directory or a parent containing result JSON files")
    parser.add_argument("output", type=Path, help="Output website directory")
    parser.add_argument("--platform", default="Raspberry Pi", help="Platform display name")
    parser.add_argument("--transport", default="tcp", choices=("tcp", "udp"), help="Carrier transport")
    parser.add_argument("--title", default="STCP Raspberry benchmark", help="Page title")
    parser.add_argument("--commit", default=os.getenv("GIT_COMMIT", "unknown"), help="Git commit identifier")
    parser.add_argument("--kernel", default=os.getenv("BENCHMARK_KERNEL", "unknown"), help="Kernel version")
    parser.add_argument("--compiler", default=os.getenv("BENCHMARK_COMPILER", "unknown"), help="Compiler description")
    parser.add_argument("--copy-raw", action=argparse.BooleanOptionalAction, default=True, help="Copy raw benchmark files")
    return parser.parse_args()


def finite_number(value: Any) -> float | int | None:
    if isinstance(value, bool):
        return int(value)
    if isinstance(value, (int, float)):
        if value != value or value in (float("inf"), float("-inf")):
            return None
        return value
    return None


def load_cases(root: Path, transport: str) -> list[dict[str, Any]]:
    cases: list[dict[str, Any]] = []
    for path in sorted(root.rglob("*.json")):
        match = CASE_RE.match(path.name)
        if not match:
            continue
        try:
            raw = json.loads(path.read_text(encoding="utf-8"))
        except (OSError, json.JSONDecodeError) as exc:
            print(f"[WARN] skipping unreadable JSON {path}: {exc}", file=sys.stderr)
            continue

        prefix = match.group("prefix")
        protocol = "stcp" if prefix.startswith("stcp-") else prefix
        case_transport = raw.get("transport") or match.group("carrier") or prefix
        if case_transport != transport and not (protocol == "tls" and transport == "tcp"):
            continue

        case: dict[str, Any] = {
            "id": path.stem,
            "filename": path.name,
            "source_path": str(path),
            "protocol": protocol,
            "protocol_label": PROTOCOL_LABEL[protocol],
            "transport": case_transport,
            "clients": int(raw.get("clients", match.group("clients"))),
            "payload_bytes": int(raw.get("payload_bytes", match.group("payload"))),
            "pipeline": int(raw.get("pipeline", match.group("pipeline"))),
            "elapsed_s": finite_number(raw.get("elapsed_s")),
            "operations": int(raw.get("operations", 0) or 0),
            "errors": int(raw.get("errors", 0) or 0),
            "error_details": list(raw.get("error_details") or []),
            "status": "pass" if int(raw.get("errors", 0) or 0) == 0 else "fail",
        }
        for metric in METRICS:
            case[metric] = finite_number(raw.get(metric))
        cases.append(case)

    cases.sort(key=lambda c: (c["payload_bytes"], c["clients"], c["pipeline"], PROTOCOL_ORDER[c["protocol"]]))
    if not cases:
        raise SystemExit(f"No benchmark result JSON files found under {root}")
    return cases


def geometric_mean(values: Iterable[float]) -> float | None:
    vals = [v for v in values if isinstance(v, (int, float)) and v > 0]
    if not vals:
        return None
    return statistics.geometric_mean(vals)


def aggregate(cases: list[dict[str, Any]]) -> dict[str, Any]:
    by_protocol: dict[str, list[dict[str, Any]]] = defaultdict(list)
    for case in cases:
        by_protocol[case["protocol"]].append(case)

    protocol_stats: dict[str, Any] = {}
    for protocol in ("tcp", "stcp", "tls"):
        rows = by_protocol.get(protocol, [])
        passed = [r for r in rows if r["errors"] == 0]
        metric_stats: dict[str, Any] = {}
        for metric in METRICS:
            vals = [float(r[metric]) for r in passed if isinstance(r.get(metric), (int, float))]
            metric_stats[metric] = {
                "median": statistics.median(vals) if vals else None,
                "mean": statistics.fmean(vals) if vals else None,
                "geomean": geometric_mean(vals),
                "min": min(vals) if vals else None,
                "max": max(vals) if vals else None,
            }
        protocol_stats[protocol] = {
            "label": PROTOCOL_LABEL[protocol],
            "cases": len(rows),
            "passed": len(passed),
            "failed": len(rows) - len(passed),
            "errors": sum(r["errors"] for r in rows),
            "pass_percent": (100.0 * len(passed) / len(rows)) if rows else 0.0,
            "operations": sum(r["operations"] for r in rows),
            "metrics": metric_stats,
        }

    # Pairwise STCP vs TLS / TCP using matching case dimensions and successful cases only.
    index = {(r["protocol"], r["clients"], r["payload_bytes"], r["pipeline"]): r for r in cases}
    comparisons: dict[str, Any] = {}
    for other in ("tls", "tcp"):
        rows: list[dict[str, float]] = []
        for stcp in by_protocol.get("stcp", []):
            key = (other, stcp["clients"], stcp["payload_bytes"], stcp["pipeline"])
            peer = index.get(key)
            if not peer or stcp["errors"] or peer["errors"]:
                continue
            item: dict[str, float] = {}
            for metric in ("operations_s", "combined_mib_s", "rtt_p50_ms", "rtt_p95_ms", "rtt_p99_ms", "connect_mean_ms", "client_cpu_percent", "server_perf_cycles_per_op", "server_perf_instructions_per_op"):
                a, b = stcp.get(metric), peer.get(metric)
                if isinstance(a, (int, float)) and isinstance(b, (int, float)) and b != 0:
                    item[metric] = 100.0 * (float(a) - float(b)) / float(b)
            rows.append(item)
        comparisons[f"stcp_vs_{other}"] = {
            "matched_pass_cases": len(rows),
            "median_percent_change": {
                metric: statistics.median([r[metric] for r in rows if metric in r])
                if any(metric in r for r in rows)
                else None
                for metric in ("operations_s", "combined_mib_s", "rtt_p50_ms", "rtt_p95_ms", "rtt_p99_ms", "connect_mean_ms", "client_cpu_percent", "server_perf_cycles_per_op", "server_perf_instructions_per_op")
            },
        }

    failure_types = Counter()
    failed_dimensions = Counter()
    for case in cases:
        if case["errors"]:
            failed_dimensions[f"c{case['clients']}/p{case['payload_bytes']}/q{case['pipeline']}"] += case["errors"]
            for detail in case["error_details"]:
                failure_types[str(detail)] += 1

    return {
        "total_cases": len(cases),
        "passed_cases": sum(1 for c in cases if c["errors"] == 0),
        "failed_cases": sum(1 for c in cases if c["errors"] != 0),
        "total_errors": sum(c["errors"] for c in cases),
        "protocols": protocol_stats,
        "comparisons": comparisons,
        "failure_types": [{"error": k, "count": v} for k, v in failure_types.most_common()],
        "failed_dimensions": [{"case": k, "errors": v} for k, v in failed_dimensions.most_common()],
    }


def payload_label(n: int) -> str:
    if n >= 1024 * 1024 and n % (1024 * 1024) == 0:
        return f"{n // (1024 * 1024)} MiB"
    if n >= 1024 and n % 1024 == 0:
        return f"{n // 1024} KiB"
    return f"{n} B"


def make_dashboard_data(cases: list[dict[str, Any]], summary: dict[str, Any], metadata: dict[str, Any]) -> dict[str, Any]:
    for c in cases:
        c["payload_label"] = payload_label(c["payload_bytes"])
        c.pop("source_path", None)
    return {
        "schema_version": 1,
        "metadata": metadata,
        "dimensions": {
            "protocols": [p for p in ("tcp", "stcp", "tls") if any(c["protocol"] == p for c in cases)],
            "clients": sorted({c["clients"] for c in cases}),
            "payload_bytes": sorted({c["payload_bytes"] for c in cases}),
            "pipelines": sorted({c["pipeline"] for c in cases}),
        },
        "summary": summary,
        "cases": cases,
    }


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for block in iter(lambda: f.read(1024 * 1024), b""):
            h.update(block)
    return h.hexdigest()


def write_cases_csv(path: Path, cases: list[dict[str, Any]]) -> None:
    fields = [
        "id", "protocol", "transport", "clients", "payload_bytes", "pipeline", "elapsed_s", "operations", "errors", "status",
        *METRICS,
        "error_details",
    ]
    with path.open("w", encoding="utf-8", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fields, extrasaction="ignore")
        writer.writeheader()
        for case in cases:
            row = dict(case)
            row["error_details"] = " | ".join(case["error_details"])
            writer.writerow(row)


def write_report(path: Path, data: dict[str, Any]) -> None:
    s = data["summary"]
    m = data["metadata"]
    lines = [
        f"# {m['title']}", "",
        f"Generated: {m['generated_at']}",
        f"Platform: {m['platform']}",
        f"Transport: {m['transport'].upper()}", "",
        "## Test status", "",
        "| Protocol | Cases | Passed | Failed | Pass rate | Errors |",
        "|---|---:|---:|---:|---:|---:|",
    ]
    for p in ("tcp", "stcp", "tls"):
        ps = s["protocols"].get(p)
        if not ps or ps["cases"] == 0:
            continue
        lines.append(f"| {ps['label']} | {ps['cases']} | {ps['passed']} | {ps['failed']} | {ps['pass_percent']:.1f}% | {ps['errors']} |")
    lines += ["", "## Pairwise median changes", "", "Positive throughput/ops is better; negative latency/CPU/cycles/instructions is better.", ""]
    for key, comp in s["comparisons"].items():
        lines.append(f"### {key.replace('_', ' ').upper()}")
        lines.append("")
        lines.append(f"Matched successful cases: {comp['matched_pass_cases']}")
        lines.append("")
        for metric, value in comp["median_percent_change"].items():
            if value is not None:
                lines.append(f"- {metric}: {value:+.2f}%")
        lines.append("")
    if s["failure_types"]:
        lines += ["## Known failures", ""]
        for item in s["failure_types"]:
            lines.append(f"- {item['count']} × `{item['error']}`")
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def copy_raw(input_root: Path, output_raw: Path) -> None:
    output_raw.mkdir(parents=True, exist_ok=True)
    for path in input_root.rglob("*"):
        if not path.is_file():
            continue
        if not (path.suffix in {".json", ".csv", ".md", ".log"} or path.name.endswith(".perf.csv")):
            continue
        rel = path.relative_to(input_root)
        dest = output_raw / rel
        dest.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(path, dest)


def main() -> int:
    args = parse_args()
    input_root = args.input.resolve()
    output = args.output.resolve()
    if not input_root.exists():
        raise SystemExit(f"Input path does not exist: {input_root}")

    cases = load_cases(input_root, args.transport)
    summary = aggregate(cases)
    generated_at = dt.datetime.now(dt.timezone.utc).replace(microsecond=0).isoformat()
    metadata = {
        "title": args.title,
        "platform": args.platform,
        "transport": args.transport,
        "generated_at": generated_at,
        "commit": args.commit,
        "kernel": args.kernel,
        "compiler": args.compiler,
        "source": str(input_root),
    }
    data = make_dashboard_data(cases, summary, metadata)

    output.mkdir(parents=True, exist_ok=True)
    assets_src = Path(__file__).resolve().parent / "assets"
    assets_dest = output / "assets"
    if assets_dest.exists():
        shutil.rmtree(assets_dest)
    shutil.copytree(assets_src, assets_dest)

    template = (Path(__file__).resolve().parent / "index.template.html").read_text(encoding="utf-8")
    html = template.replace("{{PAGE_TITLE}}", args.title).replace("{{PLATFORM}}", args.platform).replace("{{TRANSPORT}}", args.transport.upper())
    (output / "index.html").write_text(html, encoding="utf-8")
    (output / "dashboard-data.json").write_text(json.dumps(data, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    (output / "assets" / "data.js").write_text("window.STCP_BENCHMARK_DATA = " + json.dumps(data, separators=(",", ":"), ensure_ascii=False) + ";\n", encoding="utf-8")
    (output / "summary.json").write_text(json.dumps(summary, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    write_cases_csv(output / "cases.csv", cases)
    write_report(output / "report.md", data)

    if args.copy_raw:
        copy_raw(input_root, output / "raw")

    manifest: list[dict[str, Any]] = []
    for path in sorted(output.rglob("*")):
        if path.is_file() and path.name != "manifest.json":
            manifest.append({"path": str(path.relative_to(output)), "bytes": path.stat().st_size, "sha256": sha256(path)})
    (output / "manifest.json").write_text(json.dumps({"generated_at": generated_at, "files": manifest}, indent=2) + "\n", encoding="utf-8")

    print(f"[ OK ] Loaded {len(cases)} benchmark cases")
    print(f"[ OK ] Dashboard: {output / 'index.html'}")
    print(f"[ OK ] Summary:   {output / 'summary.json'}")
    print(f"[ OK ] CSV:       {output / 'cases.csv'}")
    print(f"[ OK ] Raw files: {output / 'raw'}" if args.copy_raw else "[INFO] Raw copy disabled")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
