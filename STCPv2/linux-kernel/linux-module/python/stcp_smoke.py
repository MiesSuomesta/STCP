#!/usr/bin/env python3
"""Deterministic STCP end-to-end echo smoke test.

The test validates one complete bind/listen/connect/accept/send/recv/echo cycle
at a time and exits immediately on the first failure. It is intentionally not
a throughput benchmark.
"""
from __future__ import annotations

import argparse
import queue
import sys
import threading
import time
from dataclasses import dataclass
from typing import Optional

from stcp_stress import NativeStcpSocket, create_stcp_socket, payload_for


@dataclass
class CaseResult:
    ok: bool
    stage: str
    detail: str = ""
    elapsed_ms: float = 0.0


class SocketGroup:
    def __init__(self) -> None:
        self._lock = threading.Lock()
        self._sockets: set[NativeStcpSocket] = set()

    def add(self, sock: NativeStcpSocket) -> NativeStcpSocket:
        with self._lock:
            self._sockets.add(sock)
        return sock

    def discard(self, sock: Optional[NativeStcpSocket]) -> None:
        if sock is None:
            return
        with self._lock:
            self._sockets.discard(sock)

    def close_all(self) -> None:
        with self._lock:
            sockets = list(self._sockets)
            self._sockets.clear()
        for sock in sockets:
            try:
                sock.close()
            except OSError:
                pass


def send_exact(sock: NativeStcpSocket, data: bytearray) -> None:
    view = memoryview(data)
    offset = 0
    while offset < len(view):
        count = sock.send(view[offset:])
        if count <= 0:
            raise ConnectionError(f"send returned {count} at {offset}/{len(view)}")
        offset += count


def recv_exact(sock: NativeStcpSocket, size: int) -> bytearray:
    output = bytearray(size)
    view = memoryview(output)
    offset = 0
    while offset < size:
        count = sock.recv_into(view[offset:], size - offset)
        if count <= 0:
            raise ConnectionError(f"recv returned {count} at {offset}/{size}")
        offset += count
    return output


def run_case(host: str, port: int, payload_size: int, timeout: float) -> CaseResult:
    sockets = SocketGroup()
    ready = threading.Event()
    results: queue.Queue[CaseResult] = queue.Queue(maxsize=1)
    payload = payload_for(payload_size & 0xFFFF, payload_size)

    def server() -> None:
        listener: Optional[NativeStcpSocket] = None
        conn: Optional[NativeStcpSocket] = None
        try:
            listener = sockets.add(create_stcp_socket(timeout))
            listener.bind((host, port))
            listener.listen(8)
            ready.set()

            conn, _ = listener.accept()
            sockets.add(conn)
            received = recv_exact(conn, payload_size)
            if received != payload:
                results.put(CaseResult(False, "server-verify", "received payload differs"))
                return

            send_exact(conn, received)
            results.put(CaseResult(True, "server-echo"))
        except BaseException as exc:
            if results.empty():
                results.put(CaseResult(False, "server", repr(exc)))
            ready.set()
        finally:
            for sock in (conn, listener):
                sockets.discard(sock)
                if sock is not None:
                    try:
                        sock.close()
                    except OSError:
                        pass

    thread = threading.Thread(target=server, name="stcp-smoke-server", daemon=False)
    thread.start()

    started = time.monotonic()
    client: Optional[NativeStcpSocket] = None
    client_result: Optional[CaseResult] = None
    try:
        if not ready.wait(timeout):
            return CaseResult(False, "server-ready", f"timeout after {timeout:.1f}s")

        if not results.empty():
            return results.get_nowait()

        client = sockets.add(create_stcp_socket(timeout))
        client.connect((host, port))
        send_exact(client, payload)
        reply = recv_exact(client, payload_size)
        if reply != payload:
            client_result = CaseResult(False, "client-verify", "echoed payload differs")
        else:
            client_result = CaseResult(True, "client-echo")
    except BaseException as exc:
        client_result = CaseResult(False, "client", repr(exc))
    finally:
        sockets.discard(client)
        if client is not None:
            try:
                client.close()
            except OSError:
                pass

    remaining = max(0.0, timeout - (time.monotonic() - started))
    thread.join(timeout=remaining)
    if thread.is_alive():
        sockets.close_all()
        thread.join(timeout=1.0)
        return CaseResult(False, "shutdown", "server thread did not exit")

    server_result = results.get_nowait() if not results.empty() else CaseResult(False, "server", "no result")
    elapsed_ms = (time.monotonic() - started) * 1000.0

    if client_result is None or not client_result.ok:
        result = client_result or CaseResult(False, "client", "no result")
        result.elapsed_ms = elapsed_ms
        return result
    if not server_result.ok:
        server_result.elapsed_ms = elapsed_ms
        return server_result

    return CaseResult(True, "complete", elapsed_ms=elapsed_ms)


def parse_sizes(value: str) -> list[int]:
    sizes: list[int] = []
    for item in value.split(","):
        size = int(item.strip(), 0)
        if size <= 0:
            raise argparse.ArgumentTypeError("payload sizes must be positive")
        sizes.append(size)
    return sizes


def main() -> int:
    parser = argparse.ArgumentParser(description="STCP deterministic echo smoke test")
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=7777)
    parser.add_argument("--sizes", type=parse_sizes, default=parse_sizes("64,4096,65536"))
    parser.add_argument("--timeout", type=float, default=5.0)
    args = parser.parse_args()

    print("=== STCP functional smoke test ===", flush=True)
    print(f"payload sizes: {', '.join(str(size) for size in args.sizes)} bytes", flush=True)
    print(f"timeout:       {args.timeout:.1f} s per case", flush=True)
    print(flush=True)

    for index, size in enumerate(args.sizes):
        port = args.port + index
        print(f"[{index + 1}/{len(args.sizes)}] {size} bytes on port {port}: ", end="", flush=True)
        result = run_case(args.host, port, size, args.timeout)
        if not result.ok:
            print("FAIL", flush=True)
            print(f"stage:  {result.stage}", file=sys.stderr, flush=True)
            print(f"detail: {result.detail}", file=sys.stderr, flush=True)
            return 1
        print(f"PASS ({result.elapsed_ms:.2f} ms)", flush=True)

    print("\nSTCP functional test: PASS", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
