#!/usr/bin/env python3
"""TCP vs TLS 1.3 vs AF_STCP echo benchmark."""
from __future__ import annotations

import argparse
import ctypes
import os
import socket
import ssl
import statistics
import struct
import sys
import threading
import time

AF_STCP = 45
STCP_PROTOCOL = 253
HEADER = struct.Struct("!II")
MAX_PAYLOAD = 64 * 1024 * 1024


class SockAddrIn(ctypes.Structure):
    _fields_ = [
        ("sin_family", ctypes.c_ushort),
        ("sin_port", ctypes.c_ushort),
        ("sin_addr", ctypes.c_uint32),
        ("sin_zero", ctypes.c_ubyte * 8),
    ]


libc = ctypes.CDLL(None, use_errno=True)
libc.bind.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_uint]
libc.bind.restype = ctypes.c_int
libc.connect.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_uint]
libc.connect.restype = ctypes.c_int
libc.accept.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_void_p]
libc.accept.restype = ctypes.c_int


def stcp_sockaddr(host: str, port: int) -> SockAddrIn:
    packed = socket.inet_aton(host)
    return SockAddrIn(
        sin_family=socket.AF_INET,
        sin_port=socket.htons(port),
        sin_addr=ctypes.c_uint32.from_buffer_copy(packed).value,
        sin_zero=(ctypes.c_ubyte * 8)(*([0] * 8)),
    )


def native_call(name: str, result: int) -> None:
    if result < 0:
        error = ctypes.get_errno()
        raise OSError(error, f"{name}: {os.strerror(error)}")


def bind_socket(sock: socket.socket, mode: str, host: str, port: int) -> None:
    if mode != "stcp":
        sock.bind((host, port))
        return
    address = stcp_sockaddr(host, port)
    native_call("STCP bind", libc.bind(sock.fileno(), ctypes.byref(address), ctypes.sizeof(address)))


def connect_socket(sock: socket.socket, mode: str, host: str, port: int) -> None:
    if mode != "stcp":
        sock.connect((host, port))
        return
    address = stcp_sockaddr(host, port)
    native_call("STCP connect", libc.connect(sock.fileno(), ctypes.byref(address), ctypes.sizeof(address)))


def accept_socket(listener: socket.socket, mode: str) -> tuple[socket.socket, object]:
    if mode != "stcp":
        return listener.accept()
    fd = libc.accept(listener.fileno(), None, None)
    native_call("STCP accept", fd)
    return socket.socket(AF_STCP, socket.SOCK_STREAM, STCP_PROTOCOL, fileno=fd), "STCP peer"


def recv_exact(sock: socket.socket, size: int) -> bytes:
    buf = bytearray(size)
    view = memoryview(buf)
    pos = 0
    while pos < size:
        got = sock.recv_into(view[pos:])
        if got == 0:
            raise ConnectionError("peer closed connection")
        pos += got
    return bytes(buf)


def make_socket(mode: str) -> socket.socket:
    if mode == "stcp":
        return socket.socket(AF_STCP, socket.SOCK_STREAM, STCP_PROTOCOL)
    return socket.socket(socket.AF_INET, socket.SOCK_STREAM, socket.IPPROTO_TCP)


def tune_socket(sock: socket.socket, mode: str, nodelay: bool, bufsize: int) -> None:
    if mode in ("tcp", "tls"):
        if nodelay:
            sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
        if bufsize:
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, bufsize)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, bufsize)


def server_connection(conn: socket.socket, verify: bool) -> None:
    try:
        while True:
            header = recv_exact(conn, HEADER.size)
            length, sequence = HEADER.unpack(header)
            if length > MAX_PAYLOAD:
                raise ValueError(f"payload too large: {length}")
            payload = recv_exact(conn, length)
            if verify and length >= 4 and payload[:4] != struct.pack("!I", sequence):
                raise ValueError("payload verification failed")
            conn.sendall(header + payload)
    except (OSError, ConnectionError, ValueError):
        pass
    finally:
        conn.close()


def run_server(args: argparse.Namespace) -> int:
    listener = make_socket(args.mode)
    tune_socket(listener, args.mode, args.nodelay, args.socket_buffer)
    listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    bind_socket(listener, args.mode, args.bind, args.port)
    listener.listen(args.backlog)

    tls_context = None
    if args.mode == "tls":
        tls_context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        tls_context.minimum_version = ssl.TLSVersion.TLSv1_3
        tls_context.load_cert_chain(args.cert, args.key)

    print(f"benchmark server: mode={args.mode} listen={args.bind}:{args.port}")
    try:
        while True:
            conn, address = accept_socket(listener, args.mode)
            tune_socket(conn, args.mode, args.nodelay, args.socket_buffer)
            if tls_context is not None:
                try:
                    conn = tls_context.wrap_socket(conn, server_side=True)
                except ssl.SSLError as exc:
                    print(f"TLS handshake failed from {address}: {exc}", file=sys.stderr)
                    conn.close()
                    continue
            threading.Thread(target=server_connection, args=(conn, args.verify), daemon=True).start()
    except KeyboardInterrupt:
        pass
    finally:
        listener.close()
    return 0


def percentile(values: list[float], fraction: float) -> float:
    if not values:
        return 0.0
    values = sorted(values)
    index = (len(values) - 1) * fraction
    low = int(index)
    high = min(low + 1, len(values) - 1)
    return values[low] + (values[high] - values[low]) * (index - low)


def client_thread(index: int, args: argparse.Namespace, barrier: threading.Barrier,
                  deadline: list[float], result: dict, lock: threading.Lock) -> None:
    tx = rx = operations = errors = 0
    samples: list[float] = []
    connect_ms = 0.0
    sock = None
    try:
        sock = make_socket(args.mode)
        tune_socket(sock, args.mode, args.nodelay, args.socket_buffer)
        # Python timeouts switch the fd to nonblocking mode.  Keep AF_STCP
        # blocking because the current kernel implementation expects a
        # synchronous connect/send/recv path.
        if args.mode != "stcp":
            sock.settimeout(args.timeout)
        started = time.perf_counter()
        connect_socket(sock, args.mode, args.host, args.port)
        if args.mode == "tls":
            if args.insecure:
                context = ssl._create_unverified_context()
            else:
                context = ssl.create_default_context(cafile=args.cafile)
            context.minimum_version = ssl.TLSVersion.TLSv1_3
            sock = context.wrap_socket(sock, server_hostname=args.server_name or args.host)
        connect_ms = (time.perf_counter() - started) * 1000.0
        barrier.wait()

        sequence = index << 24
        while time.perf_counter() < deadline[0]:
            if args.verify and args.payload >= 4:
                payload = struct.pack("!I", sequence) + b"x" * (args.payload - 4)
            else:
                payload = b"x" * args.payload
            frame = HEADER.pack(len(payload), sequence) + payload
            tick = time.perf_counter_ns()
            sock.sendall(frame)
            reply_header = recv_exact(sock, HEADER.size)
            reply_length, reply_sequence = HEADER.unpack(reply_header)
            reply = recv_exact(sock, reply_length)
            elapsed_ms = (time.perf_counter_ns() - tick) / 1_000_000.0
            if reply_sequence != sequence or reply_length != len(payload):
                raise ValueError("reply header mismatch")
            if args.verify and reply != payload:
                raise ValueError("reply payload mismatch")
            tx += len(payload)
            rx += len(reply)
            operations += 1
            if len(samples) < args.max_rtt_samples:
                samples.append(elapsed_ms)
            sequence = (sequence + 1) & 0xffffffff
    except Exception as exc:
        errors += 1
        if args.verbose or operations == 0:
            print(f"client {index}: {type(exc).__name__}: {exc}", file=sys.stderr, flush=True)
        try:
            barrier.abort()
        except Exception:
            pass
    finally:
        if sock is not None:
            try:
                sock.close()
            except OSError:
                pass
        with lock:
            result["tx"] += tx
            result["rx"] += rx
            result["ops"] += operations
            result["errors"] += errors
            result["connect"].append(connect_ms)
            result["rtt"].extend(samples)


def run_client(args: argparse.Namespace) -> int:
    result = {"tx": 0, "rx": 0, "ops": 0, "errors": 0, "connect": [], "rtt": []}
    lock = threading.Lock()
    barrier = threading.Barrier(args.clients + 1)
    deadline = [0.0]
    workers = [threading.Thread(target=client_thread,
                args=(i, args, barrier, deadline, result, lock), daemon=True)
               for i in range(args.clients)]
    for worker in workers:
        worker.start()

    # Install the deadline before releasing workers from the barrier.  The old
    # order had a race where a fast worker observed deadline=0.0 and exited
    # immediately, producing a valid-looking zero-second TLS result.
    started = time.perf_counter()
    cpu_started = time.process_time()
    deadline[0] = started + args.duration
    try:
        barrier.wait(timeout=max(30.0, args.timeout * args.clients))
    except threading.BrokenBarrierError:
        for worker in workers:
            worker.join(timeout=1.0)
        print("benchmark startup failed", file=sys.stderr)
        print(f"client startup errors: {result['errors']}", file=sys.stderr)
        return 2
    for worker in workers:
        worker.join()
    elapsed = max(time.perf_counter() - started, 1e-9)
    cpu = (time.process_time() - cpu_started) / elapsed * 100.0
    rtt = result["rtt"]

    print("=== STCP vs TCP/TLS result ===")
    print(f"mode:               {args.mode}")
    print(f"clients:            {args.clients}")
    print(f"payload:            {args.payload} bytes")
    print(f"elapsed:            {elapsed:.3f} s")
    print(f"operations:         {result['ops']}")
    print(f"errors:             {result['errors']}")
    print(f"TX throughput:      {result['tx'] / elapsed / 1048576:.2f} MiB/s")
    print(f"RX throughput:      {result['rx'] / elapsed / 1048576:.2f} MiB/s")
    print(f"combined:           {(result['tx'] + result['rx']) / elapsed / 1048576:.2f} MiB/s")
    print(f"operations/sec:     {result['ops'] / elapsed:.1f}")
    print(f"connect mean:       {statistics.fmean(result['connect']) if result['connect'] else 0.0:.3f} ms")
    print(f"RTT p50:            {percentile(rtt, 0.50):.3f} ms")
    print(f"RTT p95:            {percentile(rtt, 0.95):.3f} ms")
    print(f"RTT p99:            {percentile(rtt, 0.99):.3f} ms")
    print(f"client Python CPU:  {cpu:.1f}%")
    return 0 if result["errors"] == 0 else 1


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    commands = parser.add_subparsers(dest="command", required=True)
    for name in ("server", "client"):
        sub = commands.add_parser(name)
        sub.add_argument("--mode", choices=("tcp", "tls", "stcp"), required=True)
        sub.add_argument("--port", type=int, default=9000)
        sub.add_argument("--socket-buffer", type=int, default=0)
        sub.add_argument("--nodelay", action=argparse.BooleanOptionalAction, default=True)
        sub.add_argument("--verify", action=argparse.BooleanOptionalAction, default=False)
    srv = commands.choices["server"]
    srv.add_argument("--bind", default="0.0.0.0")
    srv.add_argument("--backlog", type=int, default=128)
    srv.add_argument("--cert")
    srv.add_argument("--key")
    cli = commands.choices["client"]
    cli.add_argument("--host", default="127.0.0.1")
    cli.add_argument("--clients", type=int, default=4)
    cli.add_argument("--payload", type=int, default=262144)
    cli.add_argument("--duration", type=float, default=30.0)
    cli.add_argument("--timeout", type=float, default=10.0)
    cli.add_argument("--insecure", action="store_true")
    cli.add_argument("--cafile")
    cli.add_argument("--server-name")
    cli.add_argument("--max-rtt-samples", type=int, default=100000)
    cli.add_argument("--verbose", action="store_true")
    args = parser.parse_args()
    if args.mode == "tls" and args.command == "server" and (not args.cert or not args.key):
        parser.error("TLS server requires --cert and --key")
    if args.command == "client" and not (0 <= args.payload <= MAX_PAYLOAD):
        parser.error("invalid payload size")
    return args


if __name__ == "__main__":
    args = parse_args()
    raise SystemExit(run_server(args) if args.command == "server" else run_client(args))
