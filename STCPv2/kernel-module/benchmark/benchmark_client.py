#!/usr/bin/env python3
from __future__ import annotations

import argparse
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
    args = parser.parse_args()

    barrier = threading.Barrier(args.clients)
    deadline = time.perf_counter() + args.duration
    results: list[dict[str, object]] = []
    lock = threading.Lock()

    def worker(worker_id: int) -> None:
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

        try:
            conn, connect_ms = open_connection(args)
            result["connect_ms"] = connect_ms
            barrier.wait()

            while time.perf_counter() < deadline:
                for _ in range(args.pipeline):
                    started = time.perf_counter()
                    conn.sendall(frame)
                    raw_length = recv_exact(conn, HEADER.size)
                    (length,) = HEADER.unpack(raw_length)
                    echoed = recv_exact(conn, length)
                    elapsed_ms = (time.perf_counter() - started) * 1000.0

                    if args.verify and echoed != payload:
                        raise ValueError("payload verification failed")

                    result["ops"] = int(result["ops"]) + 1
                    result["tx"] = int(result["tx"]) + len(payload)
                    result["rx"] = int(result["rx"]) + len(echoed)
                    samples = result["rtt"]
                    assert isinstance(samples, list)
                    if len(samples) < args.max_samples:
                        samples.append(elapsed_ms)
        except Exception as exc:
            result["errors"] = int(result["errors"]) + 1
            result["error_text"] = repr(exc)
        finally:
            if conn is not None:
                try:
                    conn.close()
                except OSError:
                    pass
            with lock:
                results.append(result)

    usage_before = resource.getrusage(resource.RUSAGE_SELF)
    started = time.perf_counter()
    threads = [threading.Thread(target=worker, args=(i,), daemon=True) for i in range(args.clients)]
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join()
    elapsed = time.perf_counter() - started
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
