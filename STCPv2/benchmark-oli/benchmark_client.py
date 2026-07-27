#!/usr/bin/env python3
from __future__ import annotations

import argparse
import ctypes
import json
import os
import resource
import select
import socket
import ssl
import statistics
import struct
import threading
import time
from pathlib import Path

AF_STCP = 45
STCP_PROTO_TCP = 253
STCP_PROTO_UDP = 254
HEADER = struct.Struct("!I")
UDP_MAGIC = b"SUDP"
UDP_HEADER = struct.Struct("!4sIHHI")
UDP_CHUNK_PAYLOAD = 1_200
UDP_FRAGMENT_TIMEOUT = 2.0
UDP_RETRIES = 3
UDP_WINDOW = 64


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


def wait_socket(conn: socket.socket, *, read: bool, deadline: float, operation: str) -> None:
    """Wait for AF_STCP readiness without switching the socket to O_NONBLOCK."""
    while True:
        remaining = deadline - time.perf_counter()
        if remaining <= 0:
            raise TimeoutError(f"{operation} timed out")
        readable, writable, exceptional = select.select(
            [conn] if read else [],
            [] if read else [conn],
            [conn],
            remaining,
        )
        if exceptional:
            raise OSError(f"{operation}: socket exception")
        if (read and readable) or (not read and writable):
            return


def send_exact(conn: socket.socket, data: bytes, timeout_s: float, use_poll: bool) -> None:
    if not use_poll:
        conn.sendall(data)
        return

    deadline = time.perf_counter() + timeout_s
    view = memoryview(data)
    sent = 0
    while sent < len(view):
        wait_socket(conn, read=False, deadline=deadline, operation="STCP send")
        count = conn.send(view[sent:])
        if count <= 0:
            raise ConnectionError("peer closed during send")
        sent += count


def recv_exact(conn: socket.socket, size: int, timeout_s: float, use_poll: bool) -> bytes:
    chunks: list[bytes] = []
    remaining = size
    deadline = time.perf_counter() + timeout_s
    while remaining:
        if use_poll:
            wait_socket(conn, read=True, deadline=deadline, operation="STCP receive")
        chunk = conn.recv(remaining)
        if not chunk:
            raise ConnectionError("peer closed")
        chunks.append(chunk)
        remaining -= len(chunk)
    return b"".join(chunks)



def udp_roundtrip(
    conn: socket.socket,
    payload: bytes,
    message_id: int,
    *,
    chunk_payload: int,
    fragment_timeout_s: float,
    retries: int,
    window_size: int,
) -> tuple[bytes, dict[str, int]]:
    """Echo a logical payload over MTU-safe UDP using a sliding window.

    A window of fragments is transmitted back-to-back. Echoes may arrive in
    any order. Only missing fragments are retransmitted after a window timeout.
    """
    if chunk_payload <= 0 or chunk_payload > 65_507 - UDP_HEADER.size:
        raise ValueError(f"invalid UDP chunk payload: {chunk_payload}")
    if fragment_timeout_s <= 0:
        raise ValueError("UDP fragment timeout must be positive")
    if retries < 0:
        raise ValueError("UDP retries cannot be negative")
    if window_size <= 0:
        raise ValueError("UDP window size must be positive")

    chunk_count = max(1, (len(payload) + chunk_payload - 1) // chunk_payload)
    if chunk_count > 0xFFFF:
        raise ValueError(f"payload requires too many UDP fragments: {chunk_count}")

    packets: list[bytes] = []
    expected_chunks: list[bytes] = []
    for chunk_index in range(chunk_count):
        offset = chunk_index * chunk_payload
        chunk = payload[offset:offset + chunk_payload]
        expected_chunks.append(chunk)
        packets.append(
            UDP_HEADER.pack(
                UDP_MAGIC,
                message_id & 0xFFFFFFFF,
                chunk_index,
                chunk_count,
                len(payload),
            ) + chunk
        )

    echoed_parts: list[bytes | None] = [None] * chunk_count
    stats = {
        "udp_fragments": chunk_count,
        "udp_datagrams_sent": 0,
        "udp_datagrams_received": 0,
        "udp_retransmits": 0,
        "udp_duplicates": 0,
        "udp_stale": 0,
        "udp_timeouts": 0,
    }

    for window_start in range(0, chunk_count, window_size):
        window_end = min(chunk_count, window_start + window_size)
        missing = set(range(window_start, window_end))

        for attempt in range(retries + 1):
            for chunk_index in sorted(missing):
                packet = packets[chunk_index]
                sent = conn.send(packet)
                if sent != len(packet):
                    raise OSError(
                        f"partial UDP datagram send: {sent}/{len(packet)}"
                    )
                stats["udp_datagrams_sent"] += 1
                if attempt > 0:
                    stats["udp_retransmits"] += 1

            deadline = time.perf_counter() + fragment_timeout_s

            while missing:
                remaining = deadline - time.perf_counter()
                if remaining <= 0:
                    stats["udp_timeouts"] += 1
                    break

                conn.settimeout(remaining)
                try:
                    echoed = conn.recv(65_535)
                except TimeoutError:
                    stats["udp_timeouts"] += 1
                    break

                stats["udp_datagrams_received"] += 1

                if len(echoed) < UDP_HEADER.size:
                    stats["udp_stale"] += 1
                    continue

                magic, echoed_id, echoed_index, echoed_count, echoed_total = (
                    UDP_HEADER.unpack_from(echoed)
                )

                if magic != UDP_MAGIC:
                    stats["udp_stale"] += 1
                    continue
                if echoed_id != (message_id & 0xFFFFFFFF):
                    stats["udp_stale"] += 1
                    continue
                if echoed_count != chunk_count or echoed_total != len(payload):
                    raise ValueError("UDP echo message metadata mismatch")
                if echoed_index < window_start or echoed_index >= window_end:
                    stats["udp_stale"] += 1
                    continue

                echoed_chunk = echoed[UDP_HEADER.size:]
                if echoed_chunk != expected_chunks[echoed_index]:
                    raise ValueError(
                        f"UDP echo fragment payload mismatch: "
                        f"{echoed_index + 1}/{chunk_count}"
                    )

                if echoed_index not in missing:
                    stats["udp_duplicates"] += 1
                    continue

                echoed_parts[echoed_index] = echoed_chunk
                missing.remove(echoed_index)

            if not missing:
                break
        else:
            missing_text = ",".join(str(index) for index in sorted(missing)[:16])
            raise TimeoutError(
                f"UDP window timed out: message={message_id & 0xFFFFFFFF} "
                f"window={window_start}-{window_end - 1} "
                f"missing={missing_text}"
            )

    if any(part is None for part in echoed_parts):
        raise ValueError("UDP reassembly contains missing fragments")

    echoed_payload = b"".join(part for part in echoed_parts if part is not None)
    if len(echoed_payload) != len(payload):
        raise ValueError(
            f"UDP reassembly length mismatch: {len(echoed_payload)} != {len(payload)}"
        )

    return echoed_payload, stats


def percentile(values: list[float], fraction: float) -> float:
    if not values:
        return 0.0
    ordered = sorted(values)
    index = min(len(ordered) - 1, round((len(ordered) - 1) * fraction))
    return ordered[index]


def open_connection(args: argparse.Namespace) -> tuple[socket.socket, float]:
    started = time.perf_counter()

    if args.mode == "stcp":
        proto = STCP_PROTO_TCP if args.transport == "tcp" else STCP_PROTO_UDP
        conn = socket.socket(AF_STCP, socket.SOCK_STREAM, proto)
        address = stcp_address(args.host, args.port)
        if libc.connect(conn.fileno(), ctypes.byref(address), ctypes.sizeof(address)) < 0:
            conn.close()
            native_error("STCP connect")
        # Keep AF_STCP blocking. Python settimeout() would set O_NONBLOCK.
    elif args.mode == "udp":
        conn = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        conn.settimeout(min(args.timeout, args.udp_fragment_timeout))
        conn.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 16 * 1024 * 1024)
        conn.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 16 * 1024 * 1024)
        conn.connect((args.host, args.port))
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
    parser.add_argument("--mode", choices=("tcp", "udp", "tls", "stcp"), required=True)
    parser.add_argument("--transport", choices=("tcp", "udp"), default="tcp", help="STCP carrier transport")
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
    parser.add_argument("--udp-chunk-payload", type=int, default=UDP_CHUNK_PAYLOAD)
    parser.add_argument("--udp-fragment-timeout", type=float, default=UDP_FRAGMENT_TIMEOUT)
    parser.add_argument("--udp-retries", type=int, default=UDP_RETRIES)
    parser.add_argument(
        "--udp-window",
        type=int,
        default=UDP_WINDOW,
        help="Raw UDP fragments sent before waiting for echoes",
    )
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
            "udp_fragments": 0,
            "udp_datagrams_sent": 0,
            "udp_datagrams_received": 0,
            "udp_retransmits": 0,
            "udp_duplicates": 0,
            "udp_stale": 0,
            "udp_timeouts": 0,
        }
        payload = bytes(((worker_id + offset) & 0xFF) for offset in range(args.payload))
        frame = HEADER.pack(len(payload)) + payload
        conn: socket.socket | None = None
        barrier_passed = False
        message_id = (worker_id << 24) & 0xFFFFFFFF

        try:
            conn, connect_ms = open_connection(args)
            result["connect_ms"] = connect_ms
            barrier.wait(timeout=args.timeout)
            barrier_passed = True

            while time.perf_counter() < deadline:
                for _ in range(args.pipeline):
                    started = time.perf_counter()
                    use_poll = args.mode == "stcp"
                    if args.mode == "udp":
                        echoed, udp_stats = udp_roundtrip(
                            conn,
                            payload,
                            message_id,
                            chunk_payload=args.udp_chunk_payload,
                            fragment_timeout_s=min(
                                args.timeout,
                                args.udp_fragment_timeout,
                            ),
                            retries=args.udp_retries,
                            window_size=args.udp_window,
                        )
                        for key, value in udp_stats.items():
                            result[key] = int(result[key]) + value
                        message_id = (message_id + 1) & 0xFFFFFFFF
                    else:
                        send_exact(conn, frame, args.timeout, use_poll)
                        raw_length = recv_exact(conn, HEADER.size, args.timeout, use_poll)
                        (length,) = HEADER.unpack(raw_length)
                        echoed = recv_exact(conn, length, args.timeout, use_poll)
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
            if not barrier_passed:
                try:
                    barrier.abort()
                except threading.BrokenBarrierError:
                    pass
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
    udp_metric_names = (
        "udp_fragments",
        "udp_datagrams_sent",
        "udp_datagrams_received",
        "udp_retransmits",
        "udp_duplicates",
        "udp_stale",
        "udp_timeouts",
    )
    udp_totals = {
        name: sum(int(item.get(name, 0)) for item in results)
        for name in udp_metric_names
    }

    output = {
        "mode": args.mode,
        "transport": args.transport if args.mode == "stcp" else args.mode,
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
        "udp_chunk_payload_bytes": args.udp_chunk_payload if args.mode == "udp" else None,
        "udp_window": args.udp_window if args.mode == "udp" else None,
        "udp_retries_per_window": args.udp_retries if args.mode == "udp" else None,
        **udp_totals,
        "udp_retransmit_percent": (
            udp_totals["udp_retransmits"]
            / udp_totals["udp_datagrams_sent"]
            * 100.0
            if udp_totals["udp_datagrams_sent"]
            else 0.0
        ),
    }

    print(json.dumps(output, indent=2))
    if args.output_json:
        Path(args.output_json).write_text(json.dumps(output, indent=2) + "\n")
    return 0 if errors == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
