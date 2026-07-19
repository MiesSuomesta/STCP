#!/usr/bin/env python3
"""Deterministic STCP end-to-end echo smoke test.

Each server case runs in a separate process. This is intentional: an unfinished
kernel socket release must not leave Python threads or descriptors behind, nor
turn a successful echo into a shutdown failure.
"""
from __future__ import annotations

import argparse
import multiprocessing as mp
import queue
import sys
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


def _server_process(
    host: str,
    port: int,
    payload_size: int,
    timeout: float,
    ready: mp.synchronize.Event,
    results: mp.queues.Queue,
) -> None:
    listener: Optional[NativeStcpSocket] = None
    conn: Optional[NativeStcpSocket] = None
    payload = payload_for(payload_size & 0xFFFF, payload_size)
    server_stage = "server-socket"
    try:
        listener = create_stcp_socket(timeout)
        server_stage = "server-bind"
        listener.bind((host, port))
        server_stage = "server-listen"
        listener.listen(8)
        ready.set()

        server_stage = "server-accept"
        results.put((None, "server-accept", "waiting"))
        conn, _ = listener.accept()
        results.put((None, "server-accept", "completed"))
        server_stage = "server-recv"
        results.put((None, "server-recv", "waiting"))
        received = recv_exact(conn, payload_size)
        results.put((None, "server-recv", f"completed {len(received)} bytes"))
        if received != payload:
            results.put((False, "server-verify", "received payload differs"))
            return

        server_stage = "server-send"
        results.put((None, "server-send", "waiting"))
        send_exact(conn, received)
        results.put((None, "server-send", f"completed {len(received)} bytes"))
        # Publish success before close(). A kernel release bug may block close,
        # but it must not hide a completed end-to-end echo operation.
        results.put((True, "server-echo", ""))
    except BaseException as exc:
        try:
            results.put((False, server_stage, repr(exc)))
        except BaseException:
            pass
        ready.set()
    finally:
        for sock in (conn, listener):
            if sock is not None:
                try:
                    sock.close()
                except OSError:
                    pass


def _stop_process(process: mp.Process, grace: float = 0.25) -> None:
    process.join(timeout=grace)
    if not process.is_alive():
        return
    process.terminate()
    process.join(timeout=1.0)
    if process.is_alive():
        process.kill()
        process.join(timeout=1.0)


def run_case(host: str, port: int, payload_size: int, timeout: float) -> CaseResult:
    # STCP is Linux-kernel-specific; fork also avoids pickling local test state.
    context = mp.get_context("fork")
    ready = context.Event()
    results = context.Queue(maxsize=32)
    process = context.Process(
        target=_server_process,
        args=(host, port, payload_size, timeout, ready, results),
        name=f"stcp-smoke-server-{port}",
        # Never let a server stuck in an uninterruptible kernel syscall keep
        # the smoke-test parent alive after a reported failure.
        daemon=True,
    )
    process.start()

    started = time.monotonic()
    payload = payload_for(payload_size & 0xFFFF, payload_size)
    client: Optional[NativeStcpSocket] = None
    client_result: Optional[CaseResult] = None
    server_result: Optional[CaseResult] = None
    server_trace: list[str] = []
    client_stage = "client-start"

    def consume_server_messages(block: bool = False, wait: float = 0.0) -> None:
        nonlocal server_result
        first = True
        while True:
            try:
                item = results.get(timeout=wait if block and first else 0.0)
            except queue.Empty:
                return
            first = False
            ok, stage, detail = item
            if ok is None:
                server_trace.append(f"{stage}: {detail}")
            else:
                server_result = CaseResult(bool(ok), str(stage), str(detail))

    try:
        if not ready.wait(timeout):
            client_result = CaseResult(False, "server-ready", f"timeout after {timeout:.1f}s")
        else:
            consume_server_messages()

            if server_result is None or server_result.ok:
                client_stage = "client-socket"
                client = create_stcp_socket(timeout)
                client_stage = "client-connect"
                client.connect((host, port))
                client_stage = "client-send"
                send_exact(client, payload)
                client_stage = "client-recv"
                reply = recv_exact(client, payload_size)
                client_stage = "client-verify"
                if reply != payload:
                    client_result = CaseResult(False, "client-verify", "echoed payload differs")
                else:
                    client_result = CaseResult(True, "client-echo")
    except BaseException as exc:
        consume_server_messages()
        trace = "; ".join(server_trace[-6:]) or "no server progress reported"
        client_result = CaseResult(False, client_stage, f"{exc!r}; server trace: {trace}")
    finally:
        if client is not None:
            try:
                client.close()
            except OSError:
                pass

    # The server should publish its result promptly after echoing. Do not wait
    # for its potentially broken kernel close/release path.
    if server_result is None:
        remaining = max(0.0, timeout - (time.monotonic() - started))
        deadline = time.monotonic() + remaining
        while server_result is None and time.monotonic() < deadline:
            consume_server_messages(block=True, wait=min(0.1, max(0.0, deadline - time.monotonic())))
        if server_result is None:
            trace = "; ".join(server_trace[-6:]) or "no server progress reported"
            server_result = CaseResult(False, "server-result", f"timeout after {timeout:.1f}s; trace: {trace}")

    _stop_process(process)

    # multiprocessing.Queue.join_thread() may wait forever when the server was
    # killed while blocked in an STCP syscall. The result has already been
    # consumed, so explicitly detach the feeder thread instead of joining it.
    try:
        results.cancel_join_thread()
        results.close()
    except (OSError, ValueError):
        pass

    elapsed_ms = (time.monotonic() - started) * 1000.0

    # A listener setup error can occur before a client is created. Report the
    # real server-side errno instead of masking it as ``client: no result``.
    if client_result is None and server_result is not None and not server_result.ok:
        server_result.elapsed_ms = elapsed_ms
        return server_result

    if client_result is None or not client_result.ok:
        detail = "no result"
        if process.exitcode not in (None, 0):
            detail += f"; server process exitcode={process.exitcode}"
        result = client_result or CaseResult(False, "client", detail)
        result.elapsed_ms = elapsed_ms
        return result
    if server_result is None or not server_result.ok:
        result = server_result or CaseResult(False, "server", "no result")
        result.elapsed_ms = elapsed_ms
        return result

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
