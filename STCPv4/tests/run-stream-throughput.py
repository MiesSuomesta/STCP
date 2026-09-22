#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, re, socket, statistics, subprocess, sys, time
from pathlib import Path

STAT_RE = re.compile(
    r"^(CLIENT-TX-send|CLIENT-TX-drain|PEER-RX|CLIENT-RX)\\s+"
    r"bytes=(\\d+)\\s+calls=(\\d+)\\s+bytes/call=([0-9.]+)\\s+"
    r"time=([0-9.]+)\\s+s\\s+throughput=([0-9.]+)\\s+MiB/s\\s+calls/s=([0-9.]+)$"
)
ROUND_RE = re.compile(r"^ROUND\\s+(\\d+)/(\\d+)$")

def parse_output(text: str) -> dict:
    rows, current_round, expected_rounds = [], None, None
    for raw in text.splitlines():
        line = raw.strip()
        rm = ROUND_RE.match(line)
        if rm:
            current_round, expected_rounds = int(rm.group(1)), int(rm.group(2))
            continue
        m = STAT_RE.match(line)
        if not m:
            continue
        if current_round is None:
            raise ValueError("metric before ROUND marker")
        tag, b, calls, bpc, sec, mibps, callsps = m.groups()
        rows.append({"round": current_round, "metric": tag, "bytes": int(b),
                     "calls": int(calls), "bytes_per_call": float(bpc),
                     "seconds": float(sec), "throughput_mib_s": float(mibps),
                     "calls_per_second": float(callsps)})
    if not rows:
        raise ValueError("no stream benchmark metrics found")
    rounds = sorted({r["round"] for r in rows})
    if expected_rounds is None or rounds != list(range(1, expected_rounds + 1)):
        raise ValueError(f"incomplete rounds: got={rounds} expected=1..{expected_rounds}")
    required = {"CLIENT-TX-send", "CLIENT-TX-drain", "PEER-RX", "CLIENT-RX"}
    for rnd in rounds:
        missing = required - {r["metric"] for r in rows if r["round"] == rnd}
        if missing:
            raise ValueError(f"round {rnd}: missing metrics: {sorted(missing)}")
    summary = {}
    for metric in sorted(required):
        vals = [r["throughput_mib_s"] for r in rows if r["metric"] == metric]
        times = [r["seconds"] for r in rows if r["metric"] == metric]
        summary[metric] = {
            "rounds": len(vals),
            "throughput_mib_s": {"median": statistics.median(vals), "min": min(vals),
                                 "max": max(vals), "mean": statistics.fmean(vals)},
            "seconds": {"median": statistics.median(times), "min": min(times),
                        "max": max(times), "mean": statistics.fmean(times)}}
    return {"schema": 1, "benchmark": "persistent-stream-throughput",
            "metric_semantics": "stcp_tcp_path_bench.c",
            "primary_tx_metric": "PEER-RX", "primary_rx_metric": "CLIENT-RX",
            "rounds": expected_rounds, "rows": rows, "summary": summary}

def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--bench", type=Path, default=Path("tests/tcp-path/stcp_tcp_path_bench"))
    ap.add_argument("--remote", required=True)
    ap.add_argument("--transport", choices=("tcp", "stcp"), default="stcp")
    ap.add_argument("--port", type=int, default=19953)
    ap.add_argument("--chunk", type=int, default=128044)
    ap.add_argument("--count", type=int, default=256)
    ap.add_argument("--rounds", type=int, default=3)
    ap.add_argument("--output", type=Path, default=Path("benchmark-results/stream-throughput/latest.json"))
    ap.add_argument("--raw-output", type=Path)
    ap.add_argument("--benchctl-output", type=Path)
    ap.add_argument("--clients", type=int, default=1)
    ap.add_argument("--payload-bytes", type=int)
    ap.add_argument("--pipeline", type=int, default=1)
    args = ap.parse_args()
    if not args.bench.exists():
        raise SystemExit(f"missing benchmark binary: {args.bench}")
    try:
        remote_ip = socket.gethostbyname(args.remote)
    except socket.gaierror as e:
        raise SystemExit(f"cannot resolve remote host {args.remote!r}: {e}")
    cmd = [str(args.bench), "client", args.transport, remote_ip, str(args.port),
           str(args.chunk), str(args.count), str(args.rounds)]
    wall0 = time.monotonic_ns()
    cp = subprocess.run(cmd, text=True, capture_output=True)
    wall_ns = time.monotonic_ns() - wall0
    if args.raw_output:
        args.raw_output.parent.mkdir(parents=True, exist_ok=True)
        args.raw_output.write_text(cp.stdout + cp.stderr, encoding="utf-8")
    if cp.returncode != 0:
        sys.stderr.write(cp.stdout); sys.stderr.write(cp.stderr); return cp.returncode
    doc = parse_output(cp.stdout)
    doc["command"] = cmd
    doc["parameters"] = {"remote": args.remote, "remote_ip": remote_ip, "transport": args.transport,
                         "port": args.port, "chunk": args.chunk,
                         "count": args.count, "bytes_per_round": args.chunk * args.count,
                         "mib_per_round": (args.chunk * args.count) / 1048576.0}
    doc["diagnostics"] = {"client_process_wall_seconds": wall_ns / 1e9,
                          "note": "diagnostic only; excluded from throughput"}
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(doc, indent=2) + "\n", encoding="utf-8")
    if args.benchctl_output:
        tx = doc["summary"]["PEER-RX"]["throughput_mib_s"]["median"]
        rx = doc["summary"]["CLIENT-RX"]["throughput_mib_s"]["median"]
        tx_seconds = doc["summary"]["PEER-RX"]["seconds"]["median"]
        rx_seconds = doc["summary"]["CLIENT-RX"]["seconds"]["median"]
        elapsed = tx_seconds + rx_seconds
        operations = args.count * args.rounds * 2
        raw = {
            "mode": args.transport,
            "clients": args.clients,
            "payload_bytes": args.payload_bytes if args.payload_bytes is not None else args.chunk,
            "pipeline": args.pipeline,
            "elapsed_s": elapsed,
            "operations": operations,
            "errors": 0,
            "error_details": [],
            "tx_mib_s": tx,
            "rx_mib_s": rx,
            "combined_mib_s": tx + rx,
            "operations_s": operations / elapsed if elapsed > 0 else 0.0,
            "connect_mean_ms": None,
            "rtt_p50_ms": None,
            "rtt_p95_ms": None,
            "rtt_p99_ms": None,
            "client_cpu_percent": None,
            "max_rss_kib": 0,
            "stream_benchmark": doc,
        }
        args.benchctl_output.parent.mkdir(parents=True, exist_ok=True)
        args.benchctl_output.write_text(json.dumps(raw, indent=2) + "\n", encoding="utf-8")

    print(f"STREAM-THROUGHPUT PASS transport={args.transport} rounds={doc['rounds']}")
    print(f"TX peer-rx median={doc['summary']['PEER-RX']['throughput_mib_s']['median']:.3f} MiB/s")
    print(f"RX client-rx median={doc['summary']['CLIENT-RX']['throughput_mib_s']['median']:.3f} MiB/s")
    print(f"TX client-send median={doc['summary']['CLIENT-TX-send']['throughput_mib_s']['median']:.3f} MiB/s")
    print(f"TX client-drain median={doc['summary']['CLIENT-TX-drain']['throughput_mib_s']['median']:.3f} MiB/s")
    print(f"JSON {args.output}")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
