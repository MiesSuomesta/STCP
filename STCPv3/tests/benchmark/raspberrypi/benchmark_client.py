#!/usr/bin/env python3
from __future__ import annotations

import argparse
from collections import deque
import ctypes
import json
import os
import resource
import socket
import ssl
import statistics
import struct
import threading
import time
from pathlib import Path

AF_STCP = 45
STCP_PROTO = 253
HEADER = struct.Struct("!I")


class SockAddrIn(ctypes.Structure):
    _fields_ = [
        ("sin_family", ctypes.c_ushort),
        ("sin_port", ctypes.c_ushort),
        ("sin_addr", ctypes.c_uint32),
        ("sin_zero", ctypes.c_ubyte * 8),
    ]


libc = ctypes.CDLL(None, use_errno=True)
libc.connect.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_uint]
libc.connect.restype = ctypes.c_int


def native_error(operation: str) -> None:
    err = ctypes.get_errno()
    raise OSError(err, f"{operation}: {os.strerror(err)}")


def stcp_address(host: str, port: int) -> SockAddrIn:
    packed = socket.inet_aton(host)
    return SockAddrIn(
        sin_family=socket.AF_INET,
        sin_port=socket.htons(port),
        sin_addr=ctypes.c_uint32.from_buffer_copy(packed).value,
        sin_zero=(ctypes.c_ubyte * 8)(*([0] * 8)),
    )


def recv_exact(conn: socket.socket, size: int) -> bytes:
    chunks: list[bytes] = []
    remaining = size
    while remaining:
        chunk = conn.recv(remaining)
        if not chunk:
            raise ConnectionError("peer closed")
        chunks.append(chunk)
        remaining -= len(chunk)
    return b"".join(chunks)


def percentile(values: list[float], fraction: float) -> float:
    if not values:
        return 0.0
    ordered = sorted(values)
    index = min(len(ordered) - 1, round((len(ordered) - 1) * fraction))
    return ordered[index]


def open_connection(args: argparse.Namespace) -> tuple[socket.socket, float]:
    started = time.perf_counter()

    if args.mode == "stcp":
        conn = socket.socket(AF_STCP, socket.SOCK_STREAM, STCP_PROTO)
        address = stcp_address(args.host, args.port)
        if libc.connect(conn.fileno(), ctypes.byref(address), ctypes.sizeof(address)) < 0:
            conn.close()
            native_error("STCP connect")
        # Keep AF_STCP blocking. Python settimeout() would set O_NONBLOCK.
    else:
        raw = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        raw.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        raw.settimeout(args.timeout)
        raw.connect((args.host, args.port))
        if args.mode == "tls":
            context = ssl.create_default_context()
            context.check_hostname = False
            context.verify_mode = ssl.CERT_NONE
            context.minimum_version = ssl.TLSVersion.TLSv1_3
            conn = context.wrap_socket(raw, server_hostname=args.host)
        else:
            conn = raw

    return conn, (time.perf_counter() - started) * 1000.0


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", choices=("tcp", "tls", "stcp"), required=True)
    parser.add_argument("--host", required=True)
    parser.add_argument("--port", type=int, required=True)
    parser.add_argument("--clients", type=int, default=4)
    parser.add_argument("--payload", type=int, default=262144)
    parser.add_argument("--duration", type=float, default=30.0)
    parser.add_argument("--pipeline", type=int, default=1)
    parser.add_argument("--timeout", type=float, default=30.0)
    parser.add_argument("--verify", action="store_true")
    parser.add_argument("--output-json")
    parser.add_argument("--max-samples", type=int, default=10000)
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("--stats-interval", type=float, default=1.0)
    args = parser.parse_args()

    if args.clients < 1:
        parser.error("--clients must be >= 1")
    if args.pipeline < 1:
        parser.error("--pipeline must be >= 1")
    if args.payload < 1:
        parser.error("--payload must be >= 1")
    if args.duration <= 0:
        parser.error("--duration must be > 0")
    if args.stats_interval <= 0:
        parser.error("--stats-interval must be > 0")

    start_barrier = threading.Barrier(args.clients)
    start_time = 0.0
    deadline = 0.0
    results: list[dict[str, object]] = []
    lock = threading.Lock()

    live_lock = threading.Lock()
    live = {
        "tx": 0,
        "rx": 0,
        "ops": 0,
        "errors": 0,
        "outstanding": 0,
        "active": 0,
        "draining": 0,
    }
    reporter_stop = threading.Event()

    def live_add(key: str, delta: int) -> None:
        with live_lock:
            live[key] = int(live[key]) + delta

    def reporter() -> None:
        previous_time = time.perf_counter()
        previous_tx = 0
        previous_rx = 0
        previous_ops = 0

        while not reporter_stop.wait(args.stats_interval):
            now = time.perf_counter()

            with live_lock:
                snap = dict(live)

            dt = now - previous_time
            tx = int(snap["tx"])
            rx = int(snap["rx"])
            ops = int(snap["ops"])

            tx_rate = (tx - previous_tx) / dt / 1048576 if dt else 0.0
            rx_rate = (rx - previous_rx) / dt / 1048576 if dt else 0.0
            ops_rate = (ops - previous_ops) / dt if dt else 0.0

            if start_time:
                run_s = now - start_time
                phase = "RUN" if now < deadline else "DRAIN"
            else:
                run_s = 0.0
                phase = "CONNECT"

            print(
                f"[{run_s:7.2f}s] {phase:<7} "
                f"TX {tx_rate:8.2f} MiB/s  "
                f"RX {rx_rate:8.2f} MiB/s  "
                f"ops/s {ops_rate:8.1f}  "
                f"out {int(snap['outstanding']):3d}  "
                f"active {int(snap['active']):2d}  "
                f"drain {int(snap['draining']):2d}  "
                f"errors {int(snap['errors'])}",
                flush=True,
            )

            previous_time = now
            previous_tx = tx
            previous_rx = rx
            previous_ops = ops

    def worker(worker_id: int) -> None:
        nonlocal start_time, deadline

        result: dict[str, object] = {
            "ops": 0,
            "errors": 0,
            "tx": 0,
            "rx": 0,
            "rtt": [],
            "connect_ms": 0.0,
        }

        payload = bytes(((worker_id + offset) & 0xFF) for offset in range(args.payload))
        frame = HEADER.pack(len(payload)) + payload
        conn: socket.socket | None = None
        rx_thread: threading.Thread | None = None
        connected_live = False

        # Per-connection bounded pipeline state.
        state_cv = threading.Condition()
        outstanding: deque[float] = deque()
        sending_done = False
        receiver_error: BaseException | None = None

        def receiver() -> None:
            nonlocal receiver_error

            try:
                while True:
                    with state_cv:
                        while not outstanding and not sending_done:
                            state_cv.wait()

                        if not outstanding and sending_done:
                            return

                    raw_length = recv_exact(conn, HEADER.size)  # type: ignore[arg-type]
                    (length,) = HEADER.unpack(raw_length)
                    echoed = recv_exact(conn, length)  # type: ignore[arg-type]

                    if args.verify and echoed != payload:
                        raise ValueError("payload verification failed")

                    finished = time.perf_counter()

                    with state_cv:
                        if not outstanding:
                            raise RuntimeError("received reply with no outstanding request")

                        sent_at = outstanding.popleft()

                        result["ops"] = int(result["ops"]) + 1
                        result["rx"] = int(result["rx"]) + len(echoed)
                        live_add("ops", 1)
                        live_add("rx", len(echoed))
                        live_add("outstanding", -1)

                        samples = result["rtt"]
                        assert isinstance(samples, list)
                        if len(samples) < args.max_samples:
                            samples.append((finished - sent_at) * 1000.0)

                        state_cv.notify_all()

            except BaseException as exc:
                with state_cv:
                    receiver_error = exc
                    state_cv.notify_all()

        try:
            conn, connect_ms = open_connection(args)
            result["connect_ms"] = connect_ms
            live_add("active", 1)
            connected_live = True

            # All clients connect before the timed section starts.
            barrier_index = start_barrier.wait()

            if barrier_index == 0:
                start_time = time.perf_counter()
                deadline = start_time + args.duration

            # Ensure every worker sees the initialized common deadline.
            start_barrier.wait()

            rx_thread = threading.Thread(
                target=receiver,
                name=f"benchmark-rx-{worker_id}",
                daemon=True,
            )
            rx_thread.start()

            while True:
                with state_cv:
                    while len(outstanding) >= args.pipeline and receiver_error is None:
                        state_cv.wait()

                    if receiver_error is not None:
                        raise receiver_error

                    if time.perf_counter() >= deadline:
                        break

                    # Reserve one pipeline slot before sendall(), so RX can
                    # safely match an immediate reply to this request.
                    sent_at = time.perf_counter()
                    outstanding.append(sent_at)
                    live_add("outstanding", 1)

                try:
                    conn.sendall(frame)
                except BaseException:
                    with state_cv:
                        if outstanding and outstanding[-1] == sent_at:
                            outstanding.pop()
                            live_add("outstanding", -1)
                        state_cv.notify_all()
                    raise

                result["tx"] = int(result["tx"]) + len(payload)
                live_add("tx", len(payload))

            # Stop creating new requests, but drain every request already sent.
            live_add("draining", 1)
            try:
                with state_cv:
                    sending_done = True
                    state_cv.notify_all()

                    while outstanding and receiver_error is None:
                        state_cv.wait()

                    if receiver_error is not None:
                        raise receiver_error
            finally:
                live_add("draining", -1)

            if rx_thread is not None:
                rx_thread.join()

        except BaseException as exc:
            result["errors"] = int(result["errors"]) + 1
            result["error_text"] = repr(exc)
            live_add("errors", 1)

            with state_cv:
                sending_done = True
                state_cv.notify_all()

        finally:
            if conn is not None:
                try:
                    conn.close()
                except OSError:
                    pass

            if rx_thread is not None and rx_thread.is_alive():
                rx_thread.join(timeout=1.0)

            if connected_live:
                live_add("active", -1)

            with lock:
                results.append(result)

    usage_before = resource.getrusage(resource.RUSAGE_SELF)
    started = time.perf_counter()

    reporter_thread: threading.Thread | None = None
    if args.verbose:
        reporter_thread = threading.Thread(
            target=reporter,
            name="benchmark-reporter",
            daemon=True,
        )
        reporter_thread.start()

    threads = [threading.Thread(target=worker, args=(i,), daemon=True) for i in range(args.clients)]
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join()

    reporter_stop.set()
    if reporter_thread is not None:
        reporter_thread.join(timeout=max(1.0, args.stats_interval * 2.0))

    finished = time.perf_counter()
    wall_elapsed = finished - started
    elapsed = finished - start_time if start_time else wall_elapsed
    usage_after = resource.getrusage(resource.RUSAGE_SELF)

    cpu_time = (
        usage_after.ru_utime - usage_before.ru_utime
        + usage_after.ru_stime - usage_before.ru_stime
    )
    cpu_percent = cpu_time / elapsed * 100.0 if elapsed else 0.0

    operations = sum(int(item["ops"]) for item in results)
    errors = sum(int(item["errors"]) for item in results)
    bytes_tx = sum(int(item["tx"]) for item in results)
    bytes_rx = sum(int(item["rx"]) for item in results)
    connect_times = [float(item["connect_ms"]) for item in results]
    rtts = [float(value) for item in results for value in item["rtt"]]  # type: ignore[index]

    output = {
        "mode": args.mode,
        "clients": args.clients,
        "payload_bytes": args.payload,
        "pipeline": args.pipeline,
        "elapsed_s": elapsed,
        "operations": operations,
        "errors": errors,
        "tx_mib_s": bytes_tx / elapsed / 1048576,
        "rx_mib_s": bytes_rx / elapsed / 1048576,
        "combined_mib_s": (bytes_tx + bytes_rx) / elapsed / 1048576,
        "operations_s": operations / elapsed,
        "connect_mean_ms": statistics.fmean(connect_times) if connect_times else 0.0,
        "rtt_p50_ms": percentile(rtts, 0.50),
        "rtt_p95_ms": percentile(rtts, 0.95),
        "rtt_p99_ms": percentile(rtts, 0.99),
        "client_cpu_percent": cpu_percent,
        "max_rss_kib": usage_after.ru_maxrss,
        "error_details": [item.get("error_text") for item in results if item.get("error_text")],
    }

    print(json.dumps(output, indent=2))
    if args.output_json:
        Path(args.output_json).write_text(json.dumps(output, indent=2) + "\n")
    return 0 if errors == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
