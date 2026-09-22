#!/usr/bin/env python3
import argparse
import csv
import json
import os
import platform
import re
import signal
import statistics
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

try:
    import serial
except ImportError as exc:
    raise SystemExit("pyserial missing; install requirements.txt") from exc

JSON_PART_RE = re.compile(r"STCP_BENCH_JSON_PART\s+(.*)")


def parse_args():
    p = argparse.ArgumentParser(description="STCPv2 Zephyr benchmark-v2 runner")
    p.add_argument("--serial", default=os.getenv("STCP_BENCH_SERIAL", "/dev/ttyACM0"))
    p.add_argument("--baud", type=int, default=int(os.getenv("STCP_BENCH_BAUD", "115200")))
    p.add_argument("--host", default=os.getenv("STCP_BENCH_HOST", "192.168.1.20"),
                   help="host address reachable from Zephyr")
    p.add_argument("--bind", default=os.getenv("STCP_BENCH_BIND", "0.0.0.0"))
    p.add_argument("--port", type=int, default=int(os.getenv("STCP_BENCH_PORT", "19000")),
                   help="TCP benchmark server port")
    p.add_argument("--stcp-port", type=int,
                   default=int(os.getenv("STCP_BENCH_STCP_PORT", "19010")),
                   help="STCP-TCP benchmark server port")
    p.add_argument("--total", type=int, default=int(os.getenv("STCP_BENCH_TOTAL", str(8 * 1024 * 1024))))
    p.add_argument("--chunk", type=int, default=int(os.getenv("STCP_BENCH_CHUNK", "8192")))
    p.add_argument("--timeout", type=int, default=int(os.getenv("STCP_BENCH_TIMEOUT", "60")),
                   help="per benchmark timeout in seconds")
    p.add_argument("--warmups", type=int, default=int(os.getenv("STCP_BENCH_WARMUPS", "1")))
    p.add_argument("--runs", type=int, default=int(os.getenv("STCP_BENCH_RUNS", "5")))
    p.add_argument("--transports", default=os.getenv("STCP_BENCH_TRANSPORTS", "tcp,stcp-tcp"))
    p.add_argument("--directions", default=os.getenv("STCP_BENCH_DIRECTIONS", "upload,download,full"))
    p.add_argument("--results", default=None)
    return p.parse_args()


class ZephyrShell:
    def __init__(self, device, baud):
        self.ser = serial.Serial(device, baud, timeout=0.1)
        # Give USB CDC/UART a moment to settle after open.  Do not assume the
        # shell is already sitting at a prompt: a previous full-run may have
        # left a command active or asynchronous output in flight.
        time.sleep(0.25)
        self.ser.reset_input_buffer()

    def close(self):
        if self.ser:
            self.ser.close()
            self.ser = None

    def read_until(self, predicate, timeout):
        end = time.monotonic() + timeout
        text = ""
        while time.monotonic() < end:
            chunk = self.ser.read(4096)
            if chunk:
                text += chunk.decode("utf-8", errors="replace")
                if predicate(text):
                    return text
            else:
                time.sleep(0.01)
        raise TimeoutError(f"serial timeout after {timeout}s; tail:\n{text[-6000:]}")

    def drain(self, quiet=0.12, maximum=0.8):
        end = time.monotonic() + maximum
        qend = time.monotonic() + quiet
        out = ""
        while time.monotonic() < end:
            chunk = self.ser.read(4096)
            if chunk:
                out += chunk.decode("utf-8", errors="replace")
                qend = time.monotonic() + quiet
            elif time.monotonic() >= qend:
                break
            else:
                time.sleep(0.01)
        return out

    def sync(self, timeout=12.0):
        """Recover a known Zephyr shell prompt, even after an interrupted run."""
        self.drain(quiet=0.08, maximum=0.5)

        end = time.monotonic() + timeout
        text = ""
        attempt = 0
        while time.monotonic() < end:
            attempt += 1

            # Ctrl-C gets us out of a shell command left active by an earlier
            # benchmark/full-run.  The following empty line asks Zephyr shell
            # to emit a fresh prompt.
            self.ser.write(b"\x03\r\n")
            self.ser.flush()

            slice_end = min(end, time.monotonic() + 1.0)
            while time.monotonic() < slice_end:
                chunk = self.ser.read(4096)
                if chunk:
                    text += chunk.decode("utf-8", errors="replace")
                    if "stcp>" in text:
                        text += self.drain(quiet=0.04, maximum=0.2)
                        return text
                else:
                    time.sleep(0.02)

        raise TimeoutError(
            f"unable to synchronize Zephyr shell after {timeout:.1f}s "
            f"({attempt} attempts) on {self.ser.port}; tail:\n{text[-6000:]}"
        )

    def command(self, command, timeout=5.0):
        self.sync()
        self.ser.write((command + "\r\n").encode())
        self.ser.flush()
        text = self.read_until(lambda s: "stcp>" in s, timeout)
        text += self.drain(quiet=0.04, maximum=0.2)
        return text

    def benchmark(self, command, timeout):
        self.sync()
        self.ser.write((command + "\r\n").encode())
        self.ser.flush()
        text = self.read_until(lambda s: "STCP_BENCH_JSON_END" in s, timeout)
        text += self.drain(quiet=0.08, maximum=0.5)
        parts = JSON_PART_RE.findall(text)
        if not parts:
            raise RuntimeError("benchmark JSON missing\n" + text[-6000:])
        payload = "".join(p.strip() for p in parts)
        result = json.loads(payload)
        if result.get("status") != 0 or result.get("errors") != 0:
            raise RuntimeError(f"benchmark failed: {result}\n{text[-6000:]}")
        return result, text


def proc_ticks(pid):
    try:
        fields = Path(f"/proc/{pid}/stat").read_text().split()
        return int(fields[13]) + int(fields[14])
    except (OSError, ValueError, IndexError):
        return None


def git_head(start):
    try:
        return subprocess.check_output(["git", "-C", str(start), "rev-parse", "HEAD"], text=True,
                                       stderr=subprocess.DEVNULL).strip()
    except Exception:
        return None


def build_server(server_dir):
    subprocess.run(["make", "-C", str(server_dir), "clean", "all"], check=True)


def wait_ready(proc, logfile, timeout=4.0):
    end = time.monotonic() + timeout
    while time.monotonic() < end:
        if proc.poll() is not None:
            raise RuntimeError(f"benchmark server exited rc={proc.returncode}; see {logfile}")
        try:
            if "BENCH_SERVER_READY" in logfile.read_text(errors="replace"):
                return
        except OSError:
            pass
        time.sleep(0.05)
    raise TimeoutError(f"benchmark server did not become ready; see {logfile}")


def stop_process(proc):
    if not proc or proc.poll() is not None:
        return
    proc.terminate()
    try:
        proc.wait(timeout=2)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait(timeout=2)


def mib_s(result, direction):
    if direction == "upload":
        return float(result.get("tx_mib_s", 0.0))
    if direction == "download":
        return float(result.get("rx_mib_s", 0.0))
    return float(result.get("combined_mib_s", 0.0))


def main():
    args = parse_args()
    root = Path(__file__).resolve().parent
    app_root = root.parent.parent
    server_dir = root / "host"
    server_bin = server_dir / "stcp-bench-server"

    transports = [x.strip() for x in args.transports.split(",") if x.strip()]
    directions = [x.strip() for x in args.directions.split(",") if x.strip()]
    valid_t = {"tcp", "stcp-tcp"}
    valid_d = {"upload", "download", "full"}
    if not transports or any(t not in valid_t for t in transports):
        raise SystemExit(f"transports must be subset of {sorted(valid_t)}")
    if not directions or any(d not in valid_d for d in directions):
        raise SystemExit(f"directions must be subset of {sorted(valid_d)}")
    if args.runs < 1 or args.warmups < 0 or args.total < 1 or args.chunk < 1:
        raise SystemExit("invalid run/warmup/total/chunk value")

    stamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    result_dir = Path(args.results) if args.results else root / "results" / stamp
    result_dir.mkdir(parents=True, exist_ok=True)

    build_server(server_dir)

    metadata = {
        "schema_version": 1,
        "started_utc": datetime.now(timezone.utc).isoformat(),
        "git_head": git_head(app_root),
        "host_uname": platform.platform(),
        "serial": args.serial,
        "baud": args.baud,
        "server_host": args.host,
        "server_bind": args.bind,
        "server_ports": {"tcp": args.port, "stcp-tcp": args.stcp_port},
        "total_bytes": args.total,
        "chunk_bytes": args.chunk,
        "warmups": args.warmups,
        "runs": args.runs,
        "transports": transports,
        "directions": directions,
    }
    (result_dir / "metadata.json").write_text(json.dumps(metadata, indent=2) + "\n")

    rows = []
    shell = None
    server = None
    server_log_handle = None

    def teardown(*_):
        nonlocal shell, server, server_log_handle
        stop_process(server)
        server = None
        if server_log_handle:
            server_log_handle.close()
            server_log_handle = None
        if shell:
            shell.close()
            shell = None

    signal.signal(signal.SIGINT, lambda s, f: (teardown(), sys.exit(130)))
    signal.signal(signal.SIGTERM, lambda s, f: (teardown(), sys.exit(143)))

    try:
        shell = ZephyrShell(args.serial, args.baud)
        for cmd in [
            f"stcp config host {args.host}",
            f"stcp config total {args.total}",
            f"stcp config chunk {args.chunk}",
            f"stcp config timeout {args.timeout * 1000}",
            "stcp config report 0",
        ]:
            out = shell.command(cmd)
            (result_dir / "setup-serial.log").open("a").write(f"$ {cmd}\n{out}\n")

        clk_tck = os.sysconf(os.sysconf_names["SC_CLK_TCK"])

        for transport in transports:
            stop_process(server)
            server = None
            if server_log_handle:
                server_log_handle.close()
                server_log_handle = None

            transport_port = args.port if transport == "tcp" else args.stcp_port

            server_log = result_dir / f"server-{transport}.log"
            server_log_handle = server_log.open("w")
            server = subprocess.Popen([
                str(server_bin), "--transport", transport,
                "--bind", args.bind, "--port", str(transport_port)
            ], stdout=server_log_handle, stderr=subprocess.STDOUT, text=True)
            wait_ready(server, server_log)

            zephyr_transport = "tcp" if transport == "tcp" else "stcp"
            out = shell.command(f"stcp config transport {zephyr_transport}")
            if f"Transport = {zephyr_transport}" not in out:
                raise RuntimeError(f"failed to select {zephyr_transport}:\n{out}")

            out = shell.command(f"stcp config port {transport_port}")
            if f"Port = {transport_port}" not in out:
                raise RuntimeError(f"failed to select port {transport_port}:\n{out}")

            print(f"[SERVER] {transport:8} {args.bind}:{transport_port}", flush=True)

            for direction in directions:
                total_iters = args.warmups + args.runs
                for idx in range(total_iters):
                    measured = idx >= args.warmups
                    run_no = idx - args.warmups + 1 if measured else idx + 1
                    phase = "run" if measured else "warmup"
                    label = f"{transport}-{direction}-{phase}-{run_no:02d}"
                    print(f"[{phase.upper():6}] {transport:8} {direction:8} {run_no}", flush=True)

                    before_ticks = proc_ticks(server.pid)
                    wall0 = time.monotonic()
                    try:
                        result, raw = shell.benchmark(f"stcp bench {direction}", args.timeout)
                        wall_elapsed = time.monotonic() - wall0
                        after_ticks = proc_ticks(server.pid)
                        cpu_pct = None
                        if before_ticks is not None and after_ticks is not None and wall_elapsed > 0:
                            cpu_pct = ((after_ticks - before_ticks) / clk_tck) / wall_elapsed * 100.0

                        (result_dir / f"serial-{label}.log").write_text(raw)
                        if measured:
                            row = dict(result)
                            row.update({
                                "bench_transport": transport,
                                "direction": direction,
                                "run": run_no,
                                "wall_elapsed_s": wall_elapsed,
                                "server_cpu_percent": cpu_pct,
                                "primary_mib_s": mib_s(result, direction),
                            })
                            rows.append(row)
                    except Exception as exc:
                        (result_dir / f"FAIL-{label}.txt").write_text(str(exc) + "\n")
                        raise

        fieldnames = sorted({k for row in rows for k in row.keys()})
        with (result_dir / "results.csv").open("w", newline="") as f:
            w = csv.DictWriter(f, fieldnames=fieldnames)
            w.writeheader()
            w.writerows(rows)
        (result_dir / "results.json").write_text(json.dumps(rows, indent=2) + "\n")

        summary = []
        for transport in transports:
            for direction in directions:
                selected = [r for r in rows if r["bench_transport"] == transport and r["direction"] == direction]
                vals = [float(r["primary_mib_s"]) for r in selected]
                cpus = [float(r["server_cpu_percent"]) for r in selected if r.get("server_cpu_percent") is not None]
                if not vals:
                    continue
                item = {
                    "transport": transport,
                    "direction": direction,
                    "runs": len(vals),
                    "median_mib_s": statistics.median(vals),
                    "min_mib_s": min(vals),
                    "max_mib_s": max(vals),
                    "median_server_cpu_percent": statistics.median(cpus) if cpus else None,
                }
                summary.append(item)

        (result_dir / "summary.json").write_text(json.dumps(summary, indent=2) + "\n")
        with (result_dir / "summary.txt").open("w") as f:
            f.write("STCPv2 benchmark-v2 summary\n")
            f.write("transport direction runs median_MiB/s min_MiB/s max_MiB/s server_CPU%\n")
            for s in summary:
                cpu = "n/a" if s["median_server_cpu_percent"] is None else f"{s['median_server_cpu_percent']:.1f}"
                line = (f"{s['transport']:9} {s['direction']:9} {s['runs']:4d} "
                        f"{s['median_mib_s']:12.3f} {s['min_mib_s']:9.3f} "
                        f"{s['max_mib_s']:9.3f} {cpu:>10}\n")
                f.write(line)

        print("\nSTCPv2 benchmark-v2 summary")
        print("transport direction runs median MiB/s    min    max  server CPU%")
        for s in summary:
            cpu = "n/a" if s["median_server_cpu_percent"] is None else f"{s['median_server_cpu_percent']:.1f}"
            print(f"{s['transport']:9} {s['direction']:9} {s['runs']:4d} "
                  f"{s['median_mib_s']:12.3f} {s['min_mib_s']:6.3f} {s['max_mib_s']:6.3f} {cpu:>11}")
        print(f"\nResults: {result_dir}")
        return 0
    finally:
        teardown()


if __name__ == "__main__":
    sys.exit(main())
