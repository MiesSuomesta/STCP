#!/usr/bin/env python3
"""Run Zephyr STCP benchmarks over the serial shell and write infra JSON files."""
from __future__ import annotations

import argparse
import json
import os
import select
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

try:
    import serial  # type: ignore
except ImportError as exc:
    raise SystemExit("pyserial is required: python3 -m pip install pyserial") from exc

MARKER = "STCP_BENCH_JSON "
BEGIN_MARKER = "STCP_BENCH_JSON_BEGIN "
PART_MARKER = "STCP_BENCH_JSON_PART "
END_MARKER = "STCP_BENCH_JSON_END"


def parse_csv_ints(value: str) -> list[int]:
    out = []
    for item in value.split(","):
        item = item.strip()
        if item:
            out.append(int(item, 0))
    if not out:
        raise argparse.ArgumentTypeError("at least one integer is required")
    return out


def parse_csv_words(value: str) -> list[str]:
    out = [x.strip() for x in value.split(",") if x.strip()]
    if not out:
        raise argparse.ArgumentTypeError("at least one value is required")
    return out


class ZephyrShell:
    def __init__(self, device: str, baud: int):
        self.serial = serial.Serial(device, baudrate=baud, timeout=0.1)
        self.buffer = ""

    def close(self) -> None:
        self.serial.close()

    def drain(self, seconds: float = 0.4) -> str:
        deadline = time.monotonic() + seconds
        chunks: list[str] = []
        while time.monotonic() < deadline:
            data = self.serial.read(4096)
            if data:
                chunks.append(data.decode("utf-8", errors="replace"))
            else:
                time.sleep(0.02)
        text = "".join(chunks)
        if text:
            sys.stdout.write(text)
            sys.stdout.flush()
        return text

    def command(self, command: str, settle: float = 0.25) -> str:
        self.serial.write((command + "\r\n").encode("utf-8"))
        self.serial.flush()
        return self.drain(settle)

    def run_case(self, command: str, timeout_s: float) -> dict:
        self.serial.reset_input_buffer()
        self.serial.write((command + "\r\n").encode("utf-8"))
        self.serial.flush()
        deadline = time.monotonic() + timeout_s
        line_buf = ""
        seen_json: dict | None = None
        multipart_active = False
        multipart_expected: int | None = None
        multipart_parts: list[str] = []

        while time.monotonic() < deadline:
            data = self.serial.read(4096)
            if not data:
                time.sleep(0.01)
                continue
            text = data.decode("utf-8", errors="replace")
            sys.stdout.write(text)
            sys.stdout.flush()
            line_buf += text.replace("\r", "")
            while "\n" in line_buf:
                line, line_buf = line_buf.split("\n", 1)
                begin_idx = line.find(BEGIN_MARKER)
                if begin_idx >= 0:
                    raw_length = line[begin_idx + len(BEGIN_MARKER):].strip()
                    try:
                        multipart_expected = int(raw_length)
                    except ValueError:
                        multipart_expected = None
                    multipart_parts = []
                    multipart_active = True
                    continue

                part_idx = line.find(PART_MARKER)
                if multipart_active and part_idx >= 0:
                    # Do not strip the payload: JSON chunks may legally end in
                    # spaces inside a quoted value.
                    multipart_parts.append(line[part_idx + len(PART_MARKER):])
                    continue

                if multipart_active and END_MARKER in line:
                    payload = "".join(multipart_parts)
                    multipart_active = False
                    if multipart_expected is not None and len(payload) != multipart_expected:
                        raise RuntimeError(
                            f"multipart JSON length mismatch: expected "
                            f"{multipart_expected}, got {len(payload)}"
                        )
                    try:
                        seen_json = json.loads(payload)
                    except json.JSONDecodeError as exc:
                        raise RuntimeError(f"invalid multipart benchmark JSON: {exc}") from exc
                    continue

                idx = line.find(MARKER)
                if idx >= 0:
                    payload = line[idx + len(MARKER):].strip()
                    try:
                        seen_json = json.loads(payload)
                    except json.JSONDecodeError:
                        continue
                if seen_json is not None and (
                    "completed successfully" in line or " failed:" in line
                ):
                    return seen_json

        if seen_json is not None:
            return seen_json
        raise TimeoutError(f"no {MARKER.strip()} result within {timeout_s:.0f}s")


def case_filename(transport: str, payload: int, direction: str) -> str:
    suffix = "" if direction == "full" else f"-{direction}"
    return f"{transport}-c1-p{payload}-q1{suffix}.json"


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--device", default="/dev/ttyACM0")
    p.add_argument("--baud", type=int, default=115200)
    p.add_argument("--host", required=True)
    p.add_argument("--port", type=int, default=19000)
    p.add_argument("--output", type=Path)
    p.add_argument("--transports", type=parse_csv_words, default=["tcp", "stcp"])
    p.add_argument("--payloads", type=parse_csv_ints, default=[4096, 8192, 16384])
    p.add_argument("--directions", type=parse_csv_words, default=["full"])
    p.add_argument("--total", type=int, default=1048576)
    p.add_argument("--device-timeout-ms", type=int, default=60000)
    p.add_argument("--case-timeout", type=float, default=180.0)
    p.add_argument("--pause", type=float, default=2.0)
    args = p.parse_args()

    allowed_transport = {"tcp", "stcp"}
    allowed_direction = {"upload", "download", "full"}
    if not set(args.transports) <= allowed_transport:
        p.error("--transports supports tcp,stcp")
    if not set(args.directions) <= allowed_direction:
        p.error("--directions supports upload,download,full")

    stamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    result_root = args.output or (Path(__file__).resolve().parents[3] / "tests/benchmark/zephyr/results") / f"zephyr-{stamp}"
    result_root.mkdir(parents=True, exist_ok=True)

    shell = ZephyrShell(args.device, args.baud)
    cases: list[dict] = []
    failures: list[dict] = []
    started = time.monotonic()

    try:
        shell.drain(1.0)
        common = [
            f"stcp config host {args.host}",
            f"stcp config port {args.port}",
            f"stcp config total {args.total}",
            f"stcp config timeout {args.device_timeout_ms}",
            "stcp config report 0",
        ]
        for command in common:
            shell.command(command)

        for transport in args.transports:
            mode_dir = result_root / transport
            mode_dir.mkdir(parents=True, exist_ok=True)
            shell.command(f"stcp config transport {transport}")

            for payload in args.payloads:
                shell.command(f"stcp config chunk {payload}")
                for direction in args.directions:
                    case_start = time.monotonic()
                    filename = case_filename(transport, payload, direction)
                    path = mode_dir / filename
                    print(f"\n[CASE] {transport} direction={direction} payload={payload}")
                    try:
                        result = shell.run_case(
                            f"stcp bench {direction}", args.case_timeout
                        )
                        result.update(
                            {
                                "result_file": str(path),
                                "target_host": args.host,
                                "target_port": args.port,
                                "serial_device": args.device,
                                "board": "nrf9151dk/nrf9151/ns",
                                "shield": "seeed_w5500",
                                "timestamp_utc": datetime.now(timezone.utc).isoformat(),
                                "case_wall_s": round(time.monotonic() - case_start, 3),
                            }
                        )
                        path.write_text(json.dumps(result, indent=2) + "\n")
                        cases.append(result)
                        if result.get("errors", 0) or result.get("status", 0) != 0:
                            failures.append(result)
                            print(f"[FAIL] {path}")
                        else:
                            print(f"[PASS] {path}")
                    except Exception as exc:
                        failure = {
                            "mode": transport,
                            "transport": transport,
                            "direction": direction,
                            "clients": 1,
                            "payload_bytes": payload,
                            "pipeline": 1,
                            "total_bytes": args.total,
                            "errors": 1,
                            "status": -1,
                            "error": str(exc),
                            "result_file": str(path),
                            "timestamp_utc": datetime.now(timezone.utc).isoformat(),
                        }
                        path.write_text(json.dumps(failure, indent=2) + "\n")
                        cases.append(failure)
                        failures.append(failure)
                        print(f"[FAIL] {path}: {exc}")
                    time.sleep(args.pause)
    finally:
        shell.close()

    summary = {
        "schema_version": 2,
        "platform": "zephyr-nrf9151",
        "carrier": "ethernet",
        "board": "nrf9151dk/nrf9151/ns",
        "shield": "seeed_w5500",
        "started_utc": datetime.now(timezone.utc).isoformat(),
        "elapsed_s": round(time.monotonic() - started, 3),
        "cases_total": len(cases),
        "cases_passed": len(cases) - len(failures),
        "cases_failed": len(failures),
        "host": args.host,
        "port": args.port,
        "payloads": args.payloads,
        "transports": args.transports,
        "directions": args.directions,
        "total_bytes": args.total,
        "cases": cases,
    }
    (result_root / "pipeline-summary.json").write_text(
        json.dumps(summary, indent=2) + "\n"
    )
    with (result_root / "FAILED-ZEPHYR-CASES.tsv").open("w") as f:
        f.write("result_file\ttransport\tdirection\tpayload\tstatus\terror\n")
        for item in failures:
            f.write(
                f"{item.get('result_file','')}\t{item.get('transport','')}\t"
                f"{item.get('direction','')}\t{item.get('payload_bytes','')}\t"
                f"{item.get('status','')}\t{item.get('error','')}\n"
            )

    print(
        f"[SUMMARY] total={len(cases)} passed={len(cases)-len(failures)} "
        f"failed={len(failures)} output={result_root}"
    )
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
