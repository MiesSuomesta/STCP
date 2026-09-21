#!/usr/bin/env python3
"""STCP Benchmark v2: immutable run metadata, validation, DB and reports."""
from __future__ import annotations

import argparse
import csv
import hashlib
import html
import json
import os
import platform
import shutil
import sqlite3
import subprocess
import sys
from datetime import datetime, timezone
from pathlib import Path

SCHEMA_VERSION = 1
REQUIRED_RESULT = (
    "case_id", "mode", "clients", "payload_bytes", "pipeline", "elapsed_s",
    "operations", "errors", "error_details", "tx_mib_s", "rx_mib_s",
    "combined_mib_s", "operations_s", "connect_mean_ms", "rtt_p50_ms",
    "rtt_p95_ms", "rtt_p99_ms", "client_cpu_percent", "max_rss_kib",
)

def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat()

def run_text(*command: str) -> str | None:
    try:
        return subprocess.run(command, check=True, text=True, stdout=subprocess.PIPE,
                              stderr=subprocess.DEVNULL).stdout.strip()
    except (OSError, subprocess.CalledProcessError):
        return None

def write_json(path: Path, value: object) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n")
    os.replace(temporary, path)

def load_json(path: Path) -> dict:
    value = json.loads(path.read_text())
    if not isinstance(value, dict):
        raise ValueError(f"{path}: expected JSON object")
    return value

def load_cases(path: Path) -> list[dict[str, str]]:
    with path.open(newline="") as source:
        rows = list(csv.DictReader((line for line in source if not line.startswith("#")), delimiter="\t"))
    required = {"case_id", "enabled", "mode", "port", "clients", "payload_bytes",
                "pipeline", "duration_s", "direction"}
    if not rows or not required.issubset(rows[0]):
        raise ValueError(f"{path}: invalid or empty case matrix")
    enabled = [row for row in rows if row["enabled"] == "1"]
    ids = [row["case_id"] for row in enabled]
    if len(ids) != len(set(ids)):
        raise ValueError(f"{path}: duplicate case_id")
    return enabled

def sha256(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for chunk in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()

def init_run(args: argparse.Namespace) -> None:
    output = args.run_dir.resolve()
    if output.exists() and any(output.iterdir()):
        raise ValueError(f"refusing non-empty run directory: {output}")
    (output / "cases").mkdir(parents=True, exist_ok=True)
    (output / "logs").mkdir()
    matrix_copy = output / "cases.tsv"
    shutil.copyfile(args.cases, matrix_copy)
    manifest = {
        "schema_version": SCHEMA_VERSION, "run_id": output.name, "state": "running",
        "started_utc": utc_now(), "suite": args.suite, "matrix_sha256": sha256(matrix_copy),
        "source": {"git_commit": run_text("git", "rev-parse", "HEAD"),
                   "git_describe": run_text("git", "describe", "--always", "--dirty"),
                   "git_dirty": bool(run_text("git", "status", "--porcelain"))},
        "controller": {"hostname": platform.node(), "platform": platform.platform(),
                       "kernel": platform.release(), "python": platform.python_version()},
    }
    write_json(output / "run.json", manifest)
    print(output)

def record_case(args: argparse.Namespace) -> None:
    raw = load_json(args.raw)
    case = dict(raw)
    case.update({"schema_version": SCHEMA_VERSION, "case_id": args.case_id,
                 "direction": args.direction, "attempt": args.attempt,
                 "timestamp_utc": utc_now()})
    case.setdefault("error_details", [])
    case.setdefault("bytes_tx", round(float(case.get("tx_mib_s", 0)) * float(case.get("elapsed_s", 0)) * 1048576))
    case.setdefault("bytes_rx", round(float(case.get("rx_mib_s", 0)) * float(case.get("elapsed_s", 0)) * 1048576))
    missing = [key for key in REQUIRED_RESULT if key not in case]
    if missing:
        raise ValueError(f"missing result fields: {', '.join(missing)}")
    write_json(args.output, case)

def case_error(case: dict) -> str | None:
    missing = [key for key in REQUIRED_RESULT if key not in case]
    if missing: return "missing fields: " + ", ".join(missing)
    if int(case["errors"]) != 0: return f"errors={case['errors']}"
    if int(case["operations"]) <= 0: return f"operations={case['operations']}"
    if case["error_details"]: return "error_details is not empty"
    if float(case["elapsed_s"]) <= 0: return f"elapsed_s={case['elapsed_s']}"
    return None

def validate(args: argparse.Namespace) -> dict:
    run_dir = args.run_dir.resolve()
    expected = load_cases(run_dir / "cases.tsv")
    failures, results = [], []
    for definition in expected:
        path = run_dir / "cases" / f"{definition['case_id']}.json"
        if not path.is_file():
            failures.append(f"{definition['case_id']}: result missing"); continue
        try:
            result = load_json(path); error = case_error(result)
            for key in ("case_id", "mode", "clients", "payload_bytes", "pipeline", "direction"):
                expected_value: object = definition[key]
                if key in ("clients", "payload_bytes", "pipeline"): expected_value = int(expected_value)
                if result.get(key) != expected_value:
                    failures.append(f"{definition['case_id']}: {key}={result.get(key)!r}, expected {expected_value!r}")
            if error: failures.append(f"{definition['case_id']}: {error}")
            results.append(result)
        except (OSError, ValueError, json.JSONDecodeError) as exc:
            failures.append(f"{definition['case_id']}: {exc}")
    verdict = {"schema_version": SCHEMA_VERSION, "run_id": run_dir.name,
               "validated_utc": utc_now(), "expected_cases": len(expected),
               "valid_cases": len(expected) - len({item.split(":",1)[0] for item in failures}),
               "status": "PASS" if not failures else "FAIL", "failures": failures}
    write_json(run_dir / "validation.json", verdict)
    print(json.dumps(verdict, indent=2))
    if failures and not args.allow_failure: raise SystemExit(1)
    return verdict

DDL = """
PRAGMA foreign_keys=ON;
CREATE TABLE IF NOT EXISTS runs (
 run_id TEXT PRIMARY KEY, started_utc TEXT NOT NULL, suite TEXT NOT NULL,
 git_commit TEXT, git_dirty INTEGER NOT NULL, hostname TEXT NOT NULL,
 status TEXT NOT NULL, manifest_json TEXT NOT NULL
);
CREATE TABLE IF NOT EXISTS cases (
 run_id TEXT NOT NULL REFERENCES runs(run_id), case_id TEXT NOT NULL,
 mode TEXT NOT NULL, direction TEXT NOT NULL, clients INTEGER NOT NULL,
 payload_bytes INTEGER NOT NULL, pipeline INTEGER NOT NULL, elapsed_s REAL NOT NULL,
 operations INTEGER NOT NULL, errors INTEGER NOT NULL, bytes_tx INTEGER NOT NULL,
 bytes_rx INTEGER NOT NULL, tx_mib_s REAL NOT NULL, rx_mib_s REAL NOT NULL,
 combined_mib_s REAL NOT NULL, operations_s REAL NOT NULL, connect_mean_ms REAL,
 rtt_p50_ms REAL, rtt_p95_ms REAL, rtt_p99_ms REAL, client_cpu_percent REAL,
 max_rss_kib INTEGER, result_json TEXT NOT NULL, PRIMARY KEY(run_id, case_id)
);
CREATE INDEX IF NOT EXISTS cases_comparison ON cases(mode, direction, clients, payload_bytes, pipeline);
"""

def ingest(args: argparse.Namespace) -> None:
    run_dir=args.run_dir.resolve(); manifest=load_json(run_dir/"run.json"); verdict=load_json(run_dir/"validation.json")
    if verdict["status"] != "PASS": raise ValueError("only a validated PASS run may be ingested")
    db=sqlite3.connect(args.database)
    with db:
        db.executescript(DDL)
        db.execute("INSERT OR REPLACE INTO runs VALUES(?,?,?,?,?,?,?,?)",
                   (manifest["run_id"],manifest["started_utc"],manifest["suite"],
                    manifest["source"].get("git_commit"),int(manifest["source"]["git_dirty"]),
                    manifest["controller"]["hostname"],verdict["status"],json.dumps(manifest,sort_keys=True)))
        db.execute("DELETE FROM cases WHERE run_id=?",(manifest["run_id"],))
        for path in sorted((run_dir/"cases").glob("*.json")):
            c=load_json(path)
            db.execute("INSERT INTO cases VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)",
                       (manifest["run_id"],c["case_id"],c["mode"],c["direction"],c["clients"],
                        c["payload_bytes"],c["pipeline"],c["elapsed_s"],c["operations"],c["errors"],
                        c["bytes_tx"],c["bytes_rx"],c["tx_mib_s"],c["rx_mib_s"],c["combined_mib_s"],
                        c["operations_s"],c.get("connect_mean_ms"),c.get("rtt_p50_ms"),c.get("rtt_p95_ms"),
                        c.get("rtt_p99_ms"),c.get("client_cpu_percent"),c.get("max_rss_kib"),
                        json.dumps(c,sort_keys=True)))
    print(f"ingested {manifest['run_id']} into {args.database}")

def report(args: argparse.Namespace) -> None:
    run_dir=args.run_dir.resolve()
    verdict=load_json(run_dir/"validation.json")
    cases=[load_json(path) for path in sorted((run_dir/"cases").glob("*.json"))]

    # Machine-readable, validated performance artifact for site-generator.
    performance = {
        "schema": 1,
        "benchmark": "stcp-performance",
        "status": verdict["status"],
        "run_id": run_dir.name,
        "cases": [{
            "case_id": c["case_id"], "mode": c["mode"], "direction": c["direction"],
            "clients": c["clients"], "payload_bytes": c["payload_bytes"],
            "pipeline": c["pipeline"], "elapsed_s": c["elapsed_s"],
            "operations": c["operations"], "tx_mib_s": c["tx_mib_s"],
            "rx_mib_s": c["rx_mib_s"], "combined_mib_s": c["combined_mib_s"],
            "operations_s": c["operations_s"], "connect_mean_ms": c.get("connect_mean_ms"),
            "rtt_p50_ms": c.get("rtt_p50_ms"), "rtt_p95_ms": c.get("rtt_p95_ms"),
            "rtt_p99_ms": c.get("rtt_p99_ms"), "client_cpu_percent": c.get("client_cpu_percent"),
            "max_rss_kib": c.get("max_rss_kib"),
        } for c in cases],
    }
    write_json(run_dir/"performance.json", performance)

    groups={}
    for c in cases:
        key=(c["direction"],c["clients"],c["payload_bytes"],c["pipeline"])
        groups.setdefault(key,{})[c["mode"]]=c
    rows=[]
    for key,modes in sorted(groups.items()):
        for mode in ("tcp","tls","stcp"):
            c=modes.get(mode)
            if not c: continue
            tls=modes.get("tls")
            speedup=(float(c["combined_mib_s"])/float(tls["combined_mib_s"])-1)*100 if tls and tls["combined_mib_s"] else None
            rows.append((key,mode,c,speedup))
    body=[]
    for key,mode,c,speedup in rows:
        body.append("<tr>"+"".join([
            f"<td>{html.escape(mode.upper())}</td>",f"<td>{key[0]}</td>",f"<td>{key[1]}</td>",
            f"<td>{key[2]}</td>",f"<td>{key[3]}</td>",f"<td>{c['combined_mib_s']:.3f}</td>",
            f"<td>{c['operations_s']:.1f}</td>",f"<td>{c['connect_mean_ms']:.3f}</td>",
            f"<td>{c['rtt_p99_ms']:.3f}</td>",f"<td>{c['client_cpu_percent']:.1f}</td>",
            f"<td>{'' if speedup is None else f'{speedup:+.1f}%'}</td>"])+"</tr>")
    document=f"""<!doctype html><html lang=en><meta charset=utf-8><meta name=viewport content='width=device-width'>
<title>STCP Benchmark {html.escape(run_dir.name)}</title><style>
body{{font:15px system-ui;margin:2rem;background:#0b1020;color:#e8edf7}}h1{{color:#63e6be}}.pass{{color:#69db7c}}.fail{{color:#ff8787}}
table{{border-collapse:collapse;width:100%;background:#151c31}}th,td{{padding:.55rem;border-bottom:1px solid #303a55;text-align:right}}th:first-child,td:first-child{{text-align:left}}
small{{color:#aeb8cc}}@media print{{body{{background:white;color:black}}table{{background:white}}}}
</style><h1>STCPv2 benchmark</h1><p>Run <code>{html.escape(run_dir.name)}</code> — <strong class='{verdict['status'].lower()}'>{verdict['status']}</strong></p>
<small>Generated {html.escape(utc_now())}; all figures are measured results from the same run.</small>
<table><thead><tr><th>Transport</th><th>Direction</th><th>Clients</th><th>Payload B</th><th>Pipeline</th><th>Combined MiB/s</th><th>ops/s</th><th>Connect ms</th><th>RTT p99 ms</th><th>CPU %</th><th>vs TLS</th></tr></thead><tbody>{''.join(body)}</tbody></table></html>"""
    report_dir=run_dir/"report"; report_dir.mkdir(exist_ok=True)
    (report_dir/"index.html").write_text(document)
    print(run_dir/"performance.json")
    print(report_dir/"index.html")

def approve(args: argparse.Namespace) -> None:
    run_dir=args.run_dir.resolve(); verdict=load_json(run_dir/"validation.json")
    if verdict["status"]!="PASS": raise ValueError("publication denied: validation is not PASS")
    if not (run_dir/"report"/"index.html").is_file(): raise ValueError("publication denied: report missing")
    if not (run_dir/"performance.json").is_file(): raise ValueError("publication denied: performance.json missing")
    manifest=load_json(run_dir/"run.json"); manifest.update({"state":"approved","completed_utc":utc_now()})
    write_json(run_dir/"run.json",manifest)
    files=[p for p in run_dir.rglob("*") if p.is_file() and p.name not in {"publish-approved.json","SHA256SUMS"}]
    sums="".join(f"{sha256(p)}  {p.relative_to(run_dir)}\n" for p in sorted(files))
    (run_dir/"SHA256SUMS").write_text(sums)
    approval={"schema_version":SCHEMA_VERSION,"run_id":run_dir.name,"approved_utc":utc_now(),
              "validation_sha256":sha256(run_dir/"validation.json"),
              "checksums_sha256":sha256(run_dir/"SHA256SUMS")}
    write_json(run_dir/"publish-approved.json",approval)
    print(run_dir/"publish-approved.json")

def parser() -> argparse.ArgumentParser:
    root=argparse.ArgumentParser(); commands=root.add_subparsers(dest="command",required=True)
    p=commands.add_parser("init"); p.add_argument("--run-dir",type=Path,required=True); p.add_argument("--cases",type=Path,required=True); p.add_argument("--suite",default="stcp-v2"); p.set_defaults(func=init_run)
    p=commands.add_parser("record-case"); p.add_argument("--raw",type=Path,required=True); p.add_argument("--output",type=Path,required=True); p.add_argument("--case-id",required=True); p.add_argument("--direction",required=True); p.add_argument("--attempt",type=int,required=True); p.set_defaults(func=record_case)
    p=commands.add_parser("validate"); p.add_argument("--run-dir",type=Path,required=True); p.add_argument("--allow-failure",action="store_true"); p.set_defaults(func=validate)
    p=commands.add_parser("ingest"); p.add_argument("--run-dir",type=Path,required=True); p.add_argument("--database",type=Path,required=True); p.set_defaults(func=ingest)
    p=commands.add_parser("report"); p.add_argument("--run-dir",type=Path,required=True); p.set_defaults(func=report)
    p=commands.add_parser("approve"); p.add_argument("--run-dir",type=Path,required=True); p.set_defaults(func=approve)
    return root

if __name__=="__main__":
    try:
        values=parser().parse_args(); values.func(values)
    except (OSError,ValueError,KeyError,json.JSONDecodeError,sqlite3.Error) as exc:
        print(f"benchmark-v2: {exc}",file=sys.stderr); raise SystemExit(2)
