#!/usr/bin/env python3
"""Run Zephyr STCP benchmarks over the serial shell and write infra JSON files."""
from __future__ import annotations

import argparse
import array
import fcntl
import json
import os
import re
import select
import shutil
import subprocess
import sys
import time
import termios
from datetime import datetime, timezone
from pathlib import Path

MARKER = "STCP_BENCH_JSON "
BEGIN_MARKER = "STCP_BENCH_JSON_BEGIN "
PART_MARKER = "STCP_BENCH_JSON_PART "
END_MARKER = "STCP_BENCH_JSON_END"


ANSI_ESCAPE_RE = re.compile(r"\x1b(?:[@-Z\-_]|\[[0-?]*[ -/]*[@-~])")
COMPACT_RESULT_RE = re.compile(
    r"STCP_RESULT\s+"
    r"dir=(?P<dir>[A-Za-z0-9_-]+)\s+"
    r"transport=(?P<transport>[A-Za-z0-9_-]+)\s+"
    r"payload=(?P<payload>\d+)\s+"
    r"total=(?P<total>\d+)\s+"
    r"elapsed_ms=(?P<elapsed_ms>-?\d+)\s+"
    r"operations=(?P<operations>\d+)\s+"
    r"status=(?P<status>-?\d+)\s+"
    r"errors=(?P<errors>\d+)\s+"
    r"tx=(?P<tx>\d+)\s+"
    r"rx=(?P<rx>\d+)"
)


def parse_csv_ints(value: str) -> list[int]:
    out = [int(item.strip(), 0) for item in value.split(",") if item.strip()]
    if not out:
        raise argparse.ArgumentTypeError("at least one integer is required")
    return out


def parse_csv_words(value: str) -> list[str]:
    out = [item.strip() for item in value.split(",") if item.strip()]
    if not out:
        raise argparse.ArgumentTypeError("at least one value is required")
    return out


def case_filename(transport: str, payload: int, direction: str) -> str:
    """Return deterministic infra-compatible result filename."""
    safe_transport = transport.strip().lower().replace("/", "-")
    safe_direction = direction.strip().lower().replace("/", "-")
    return f"{safe_transport}-c1-p{payload}-q1-{safe_direction}.json"


def compact_result_to_json(fields: dict[str, str]) -> dict:
    """Expand one short STCP_RESULT record to the infra JSON schema."""
    direction = fields["dir"]
    transport = fields["transport"]
    payload = int(fields["payload"])
    total = int(fields["total"])
    elapsed_ms = int(fields["elapsed_ms"])
    operations = int(fields["operations"])
    status = int(fields["status"])
    errors = int(fields["errors"])
    bytes_tx = int(fields["tx"])
    bytes_rx = int(fields["rx"])

    elapsed_s = elapsed_ms / 1000.0 if elapsed_ms > 0 else 0.0
    tx_mib_s = (bytes_tx / 1048576.0) / elapsed_s if elapsed_s > 0 else 0.0
    rx_mib_s = (bytes_rx / 1048576.0) / elapsed_s if elapsed_s > 0 else 0.0
    operations_s = operations / elapsed_s if elapsed_s > 0 else 0.0

    return {
        "schema_version": 2,
        "platform": "zephyr-nrf9151",
        "carrier": "ethernet",
        "mode": transport,
        "transport": transport,
        "direction": direction,
        "clients": 1,
        "payload_bytes": payload,
        "pipeline": 1,
        "total_bytes": total,
        "elapsed_s": round(elapsed_s, 6),
        "elapsed_ms": elapsed_ms,
        "operations": operations,
        "errors": errors,
        "status": status,
        "bytes_tx": bytes_tx,
        "bytes_rx": bytes_rx,
        "tx_mib_s": round(tx_mib_s, 6),
        "rx_mib_s": round(rx_mib_s, 6),
        "combined_mib_s": round(tx_mib_s + rx_mib_s, 6),
        "operations_s": round(operations_s, 6),
        "connect_mean_ms": None,
        "rtt_p50_ms": None,
        "rtt_p95_ms": None,
        "rtt_p99_ms": None,
        "client_cpu_percent": None,
    }


def reset_board(serial_number: str, settle_s: float = 2.0) -> None:
    """Reset target before opening the CDC console.

    The J-Link UART bridge normally remains enumerated during a target reset,
    so do not wait for /dev/ttyACM0 to disappear.
    """
    nrfutil = shutil.which("nrfutil")
    if not nrfutil:
        raise RuntimeError("nrfutil not found in PATH; use --skip-reset")

    command = [nrfutil, "device", "reset", "--reset-kind", "RESET_PIN", "--serial-number", serial_number]
    print(f"[INFO] Resetting board {serial_number} with nrfutil ...", flush=True)
    completed = subprocess.run(command, text=True, capture_output=True, timeout=30)
    if completed.returncode != 0:
        details = (completed.stderr or completed.stdout).strip()
        raise RuntimeError(f"nrfutil reset failed ({completed.returncode}): {details}")
    print("[OK] Board reset requested", flush=True)
    time.sleep(max(settle_s, 0.5))


class ZephyrShell:
    """Minimal raw POSIX serial transport for Zephyr CDC ACM.

    This deliberately avoids pyserial.  On this host, pyserial's CDC ACM
    timeout/DTR handling prevented the shell runner from receiving replies,
    while Minicom worked on the same /dev/ttyACM0 device.
    """

    def __init__(self, device: str, baud: int, serial_log: Path):
        flags = os.O_RDWR | os.O_NOCTTY | os.O_NONBLOCK
        self.fd = os.open(device, flags)
        self.port = device
        self._configure_tty(baud)
        self._set_dtr(True)
        self.serial_log = serial_log.open("w", buffering=1, encoding="utf-8")
        time.sleep(0.50)

    def _set_dtr(self, enabled: bool) -> None:
        """Explicitly control CDC ACM DTR without pyserial.

        Zephyr USB CDC consoles commonly gate console traffic on DTR.  Minicom
        raises DTR automatically; a raw os.open()/termios client must do it
        explicitly.
        """
        bit = array.array("i", [termios.TIOCM_DTR])
        request = termios.TIOCMBIS if enabled else termios.TIOCMBIC
        fcntl.ioctl(self.fd, request, bit, True)

    def _configure_tty(self, baud: int) -> None:
        speeds = {
            9600: termios.B9600,
            19200: termios.B19200,
            38400: termios.B38400,
            57600: termios.B57600,
            115200: termios.B115200,
        }
        if baud not in speeds:
            raise ValueError(f"unsupported baud rate: {baud}")

        attrs = termios.tcgetattr(self.fd)
        attrs[0] = 0  # iflag: raw input, no software flow control
        attrs[1] = 0  # oflag: no output processing
        attrs[2] = termios.CLOCAL | termios.CREAD | termios.CS8
        attrs[3] = 0  # lflag: non-canonical, no echo, no signals
        attrs[4] = speeds[baud]
        attrs[5] = speeds[baud]
        attrs[6][termios.VMIN] = 0
        attrs[6][termios.VTIME] = 0
        termios.tcsetattr(self.fd, termios.TCSANOW, attrs)
        termios.tcflush(self.fd, termios.TCIOFLUSH)

    def close(self) -> None:
        # Keep DTR asserted. Lowering DTR here left the Zephyr/J-Link CDC
        # console unresponsive until a physical reset on the next run.
        try:
            self.serial_log.close()
        finally:
            os.close(self.fd)

    def _show(self, text: str) -> None:
        if not text:
            return
        sys.stdout.write(text)
        sys.stdout.flush()
        self.serial_log.write(text)

    def _read_available(self) -> bytes:
        readable, _, _ = select.select([self.fd], [], [], 0.10)
        if not readable:
            return b""
        try:
            return os.read(self.fd, 4096)
        except BlockingIOError:
            return b""

    def _read_for(self, seconds: float) -> str:
        deadline = time.monotonic() + seconds
        chunks: list[str] = []
        while time.monotonic() < deadline:
            data = self._read_available()
            if data:
                text = data.decode("utf-8", errors="replace")
                chunks.append(text)
                self._show(text)
        return "".join(chunks)

    def drain(self, seconds: float = 0.4) -> str:
        return self._read_for(seconds)

    def _write_bytes(self, data: bytes) -> None:
        view = memoryview(data)
        deadline = time.monotonic() + 2.0
        while view:
            try:
                count = os.write(self.fd, view)
                view = view[count:]
            except BlockingIOError:
                if time.monotonic() >= deadline:
                    raise TimeoutError("serial write timed out")
                select.select([], [self.fd], [], 0.05)
        termios.tcdrain(self.fd)

    def _write_command(self, command: str) -> None:
        # Zephyr shell accepts CR as Enter.  Send a leading CR to redraw prompt.
        self._write_bytes(command.encode("utf-8") + b"\r")

    def _recover_shell_line(self) -> None:
        # Wake the terminal, cancel a possible partial line, then redraw prompt.
        self._write_bytes(b"\r\x03\r")
        time.sleep(0.10)

    def wait_until_ready(self, timeout_s: float = 40.0) -> None:
        print(f"[INFO] Waiting for Zephyr shell on {self.port} ...")
        deadline = time.monotonic() + timeout_s
        collected = ""
        next_probe = 0.0
        self._recover_shell_line()

        while time.monotonic() < deadline:
            now = time.monotonic()
            if now >= next_probe:
                self._write_command("stcp config show")
                next_probe = now + 1.0

            data = self._read_available()
            if not data:
                continue
            text = data.decode("utf-8", errors="replace")
            self._show(text)
            collected = (collected + text.replace("\r", ""))[-16384:]
            if "Host      :" in collected and "Transport :" in collected:
                print("[OK] Zephyr STCP shell is ready")
                return

        raise TimeoutError(
            f"Zephyr shell did not answer 'stcp config show' within {timeout_s:.0f}s"
        )

    def command_expect(self, command: str, expected: str, timeout_s: float = 5.0) -> str:
        self._write_command(command)
        deadline = time.monotonic() + timeout_s
        received = ""
        while time.monotonic() < deadline:
            data = self._read_available()
            if not data:
                continue
            text = data.decode("utf-8", errors="replace")
            self._show(text)
            received += text.replace("\r", "")
            if expected in received:
                return received
            if "wrong parameter count" in received or "command not found" in received:
                raise RuntimeError(f"device rejected command: {command}")
        raise TimeoutError(f"no acknowledgement '{expected}' for command: {command}")

    def run_case(self, command: str, timeout_s: float) -> dict:
        self.drain(0.15)
        self._write_command(command)
        deadline = time.monotonic() + timeout_s
        line_buf = ""

        # New robust multipart format. Parts are keyed by sequence number, so
        # duplicate UART lines are harmless and missing lines can be recovered
        # from the firmware's repeated transmissions.
        numbered_expected: int | None = None
        numbered_length: int | None = None
        numbered_parts: dict[int, str] = {}

        # Compatibility with older unnumbered multipart firmware.
        legacy_active = False
        legacy_expected: int | None = None
        legacy_parts: list[str] = []

        while time.monotonic() < deadline:
            data = self._read_available()
            if not data:
                continue
            text = data.decode("utf-8", errors="replace")
            self._show(text)
            line_buf += text.replace("\r", "")

            while "\n" in line_buf:
                line, line_buf = line_buf.split("\n", 1)

                clean_line = ANSI_ESCAPE_RE.sub("", line).replace("\\n", "")
                compact_match = COMPACT_RESULT_RE.search(clean_line)
                if compact_match is not None:
                    return compact_result_to_json(compact_match.groupdict())

                begin_idx = line.find(BEGIN_MARKER)
                if begin_idx >= 0:
                    fields = line[begin_idx + len(BEGIN_MARKER):].strip().split()
                    if fields:
                        try:
                            announced_length = int(fields[0])
                        except ValueError:
                            announced_length = None
                        announced_count = None
                        if len(fields) >= 2:
                            try:
                                announced_count = int(fields[1])
                            except ValueError:
                                announced_count = None

                        if announced_count is not None:
                            # Do not clear already collected parts when a
                            # repeated BEGIN for the same frame arrives.
                            if (numbered_expected is not None and
                                    announced_count != numbered_expected):
                                numbered_parts.clear()
                            numbered_expected = announced_count
                            numbered_length = announced_length
                        else:
                            legacy_active = True
                            legacy_expected = announced_length
                            legacy_parts = []
                    continue

                part_idx = line.find(PART_MARKER)
                if part_idx >= 0:
                    payload = line[part_idx + len(PART_MARKER):]
                    head, sep, body = payload.partition(" ")
                    if sep and "/" in head:
                        idx_text, count_text = head.split("/", 1)
                        try:
                            idx = int(idx_text)
                            count = int(count_text)
                        except ValueError:
                            idx = -1
                            count = -1
                        if idx >= 0 and count > 0 and idx < count:
                            if numbered_expected is not None and count != numbered_expected:
                                numbered_parts.clear()
                            numbered_expected = count
                            # Keep the first copy. Duplicate mirrored UART
                            # lines must not be concatenated.
                            numbered_parts.setdefault(idx, body)
                        continue

                    # Legacy unnumbered PART support.
                    if not legacy_active:
                        legacy_active = True
                        legacy_expected = None
                        legacy_parts = []
                    legacy_parts.append(payload)
                    continue

                if END_MARKER in line:
                    if numbered_expected is not None:
                        missing = [i for i in range(numbered_expected)
                                   if i not in numbered_parts]
                        if not missing:
                            payload = "".join(numbered_parts[i]
                                              for i in range(numbered_expected))
                            if (numbered_length is not None and
                                    len(payload) != numbered_length):
                                # A corrupted duplicate may have reached the
                                # map first. Wait for another repeated frame
                                # rather than returning bad JSON.
                                continue
                            try:
                                return json.loads(payload)
                            except json.JSONDecodeError:
                                # Continue collecting repeated parts until the
                                # case timeout.
                                continue
                        # END arrived with missing parts. Firmware repeats the
                        # frame, so keep what we have and wait for more.
                        continue

                    if legacy_active:
                        payload = "".join(legacy_parts)
                        legacy_active = False
                        if legacy_expected is not None and len(payload) != legacy_expected:
                            raise RuntimeError(
                                "multipart JSON length mismatch: "
                                f"expected {legacy_expected}, got {len(payload)}"
                            )
                        try:
                            return json.loads(payload)
                        except json.JSONDecodeError as exc:
                            raise RuntimeError(
                                "incomplete legacy benchmark JSON received: "
                                f"{exc}"
                            ) from exc

                marker_idx = line.find(MARKER)
                if marker_idx >= 0:
                    payload = line[marker_idx + len(MARKER):].strip()
                    return json.loads(payload)

        if numbered_expected is not None:
            missing = [i for i in range(numbered_expected) if i not in numbered_parts]
            raise TimeoutError(
                "case timed out waiting for complete benchmark JSON: "
                f"{command}; received {len(numbered_parts)}/{numbered_expected} "
                f"parts, missing={missing}"
            )
        raise TimeoutError(f"case timed out waiting for benchmark JSON: {command}")


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--device", default="/dev/ttyACM0")
    p.add_argument("--baud", type=int, default=115200)
    p.add_argument("--host", required=True)
    p.add_argument("--port", type=int, default=19000)
    p.add_argument("--output", type=Path)
    p.add_argument("--transports", type=parse_csv_words, default=["tcp", "stcp"])
    p.add_argument("--payloads", type=parse_csv_ints, default=[4096, 8192, 16384])
    p.add_argument(
        "--max-chunk",
        type=int,
        default=65536,
        help="maximum physical device chunk; larger logical payloads are segmented",
    )
    p.add_argument("--directions", type=parse_csv_words, default=["full"])
    p.add_argument("--total", type=int, default=1048576)
    p.add_argument("--device-timeout-ms", type=int, default=60000)
    p.add_argument("--case-timeout", type=float, default=180.0)
    p.add_argument("--ready-timeout", type=float, default=40.0)
    p.add_argument("--pause", type=float, default=2.0)
    p.add_argument("--serial-number", default="1052043013")
    p.add_argument("--skip-reset", action="store_true")
    p.add_argument("--boot-wait", type=float, default=15.0)
    args = p.parse_args()

    if not set(args.transports) <= {"tcp", "stcp"}:
        p.error("--transports supports tcp,stcp")
    if not set(args.directions) <= {"upload", "download", "full"}:
        p.error("--directions supports upload,download,full")
    if args.max_chunk < 1 or args.max_chunk > 65536:
        p.error("--max-chunk must be in range 1..65536")
    if any(payload < 1 for payload in args.payloads):
        p.error("all payloads must be positive")

    stamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    result_root = args.output or Path("benchmark/results") / f"zephyr-{stamp}"
    result_root.mkdir(parents=True, exist_ok=True)

    if not args.skip_reset:
        reset_board(args.serial_number)

    shell = ZephyrShell(args.device, args.baud, result_root / "serial.log")
    print(f"[INFO] Waiting {args.boot_wait:.0f}s for Zephyr boot on {args.device} ...", flush=True)
    shell.drain(args.boot_wait)
    cases: list[dict] = []
    failures: list[dict] = []
    started = time.monotonic()
    started_utc = datetime.now(timezone.utc).isoformat()

    try:
        shell.wait_until_ready(args.ready_timeout)
        shell.command_expect(f"stcp config host {args.host}", f"Host = {args.host}")
        shell.command_expect(f"stcp config port {args.port}", f"Port = {args.port}")
        shell.command_expect(f"stcp config total {args.total}", f"Total = {args.total}")
        shell.command_expect(
            f"stcp config timeout {args.device_timeout_ms}",
            f"Timeout = {args.device_timeout_ms} ms",
        )
        shell.command_expect("stcp config report 0", "Report interval = 0 ms")

        for transport in args.transports:
            mode_dir = result_root / transport
            mode_dir.mkdir(parents=True, exist_ok=True)
            shell.command_expect(
                f"stcp config transport {transport}", f"Transport = {transport}"
            )

            for payload in args.payloads:
                effective_chunk = min(payload, args.max_chunk)
                if effective_chunk != payload:
                    print(
                        f"[INFO] logical payload={payload} uses device chunk={effective_chunk}",
                        flush=True,
                    )
                shell.command_expect(
                    f"stcp config chunk {effective_chunk}",
                    f"Chunk = {effective_chunk}",
                )
                for direction in args.directions:
                    case_start = time.monotonic()
                    path = mode_dir / case_filename(transport, payload, direction)
                    print(f"\n[CASE] {transport} direction={direction} payload={payload}")
                    try:
                        result = shell.run_case(f"stcp bench {direction}", args.case_timeout)

                        # Firmware reports the physical I/O chunk. Preserve it separately,
                        # but expose the requested logical payload for Raspberry Pi/x86
                        # matrix compatibility.
                        device_payload = int(result.get("payload_bytes", effective_chunk))
                        logical_operations = (
                            args.total + payload - 1
                        ) // payload
                        elapsed_s = float(result.get("elapsed_s", 0.0) or 0.0)
                        result["device_payload_bytes"] = device_payload
                        result["chunk_bytes"] = effective_chunk
                        result["payload_bytes"] = payload
                        result["operations"] = logical_operations
                        result["operations_s"] = round(
                            logical_operations / elapsed_s, 6
                        ) if elapsed_s > 0 else 0.0

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
                            "chunk_bytes": effective_chunk,
                            "device_payload_bytes": effective_chunk,
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
        "started_utc": started_utc,
        "finished_utc": datetime.now(timezone.utc).isoformat(),
        "elapsed_s": round(time.monotonic() - started, 3),
        "cases_total": len(cases),
        "cases_passed": len(cases) - len(failures),
        "cases_failed": len(failures),
        "host": args.host,
        "port": args.port,
        "payloads": args.payloads,
        "max_chunk_bytes": args.max_chunk,
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
