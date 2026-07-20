#!/usr/bin/env python3
"""
STCP kernel-module stress and throughput test.

Runs an in-process STCP echo server and configurable client workers.
Reports progress periodically and writes a JSON summary.

Examples:
    sudo ./stcp_stress.py --mode throughput --clients 16 --duration 180
    sudo ./stcp_stress.py --udp --mode throughput --clients 16 --duration 180
    sudo ./stcp_stress.py --mode churn --clients 32 --duration 60
    sudo ./stcp_stress.py --mode mixed --clients 16 --duration 120
"""

from __future__ import annotations

import argparse
import collections
import ctypes
import fcntl
import dataclasses
import errno
import json
import math
import os
import signal
import select
import socket
import statistics
import sys
import threading
import time
from pathlib import Path
from typing import Deque, Iterable, Optional

AF_STCP = 45
STCP_PROTOCOL_TCP = 253
STCP_PROTOCOL_UDP = 254
DEFAULT_HOST = "127.0.0.1"
DEFAULT_PORT = 7777


@dataclasses.dataclass
class Counters:
    connections_ok: int = 0
    connections_failed: int = 0
    sends_ok: int = 0
    sends_failed: int = 0
    recvs_ok: int = 0
    recvs_failed: int = 0
    verify_failures: int = 0
    bytes_sent: int = 0
    bytes_received: int = 0
    server_accepts: int = 0
    server_errors: int = 0

    def copy(self) -> "Counters":
        return dataclasses.replace(self)

    def delta(self, previous: "Counters") -> "Counters":
        values = {
            field.name: getattr(self, field.name) - getattr(previous, field.name)
            for field in dataclasses.fields(self)
        }
        return Counters(**values)


class SharedStats:
    def __init__(self, latency_sample_limit: int = 100_000) -> None:
        self._lock = threading.Lock()
        self.counters = Counters()
        self.latencies_ms: Deque[float] = collections.deque(
            maxlen=latency_sample_limit
        )

    def add(self, **updates: int) -> None:
        with self._lock:
            for key, value in updates.items():
                setattr(
                    self.counters,
                    key,
                    getattr(self.counters, key) + value,
                )

    def add_latency(self, latency_ms: float) -> None:
        with self._lock:
            self.latencies_ms.append(latency_ms)

    def snapshot(self) -> tuple[Counters, list[float]]:
        with self._lock:
            return self.counters.copy(), list(self.latencies_ms)




class SockAddrIn(ctypes.Structure):
    _fields_ = [
        ("sin_family", ctypes.c_ushort),
        ("sin_port", ctypes.c_ushort),
        ("sin_addr", ctypes.c_uint32),
        ("sin_zero", ctypes.c_ubyte * 8),
    ]


libc = ctypes.CDLL(None, use_errno=True)
libc.socket.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_int]
libc.socket.restype = ctypes.c_int
libc.bind.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_uint]
libc.bind.restype = ctypes.c_int
libc.listen.argtypes = [ctypes.c_int, ctypes.c_int]
libc.listen.restype = ctypes.c_int
libc.accept.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_void_p]
libc.accept.restype = ctypes.c_int
libc.connect.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_uint]
libc.connect.restype = ctypes.c_int
libc.send.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_size_t, ctypes.c_int]
libc.send.restype = ctypes.c_ssize_t
libc.recv.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_size_t, ctypes.c_int]
libc.recv.restype = ctypes.c_ssize_t
libc.close.argtypes = [ctypes.c_int]
libc.close.restype = ctypes.c_int


def _raise_errno(operation: str) -> None:
    error = ctypes.get_errno()
    raise OSError(error, f"{operation}: {os.strerror(error)}")


def _sockaddr(host: str, port: int) -> SockAddrIn:
    packed = socket.inet_aton(host)
    return SockAddrIn(
        sin_family=socket.AF_INET,
        sin_port=socket.htons(port),
        sin_addr=ctypes.c_uint32.from_buffer_copy(packed).value,
        sin_zero=(ctypes.c_ubyte * 8)(*([0] * 8)),
    )


class NativeStcpSocket:
    """Minimal libc-backed socket wrapper for an unknown Python address family."""

    def __init__(
        self,
        fd: int,
        timeout: float = 10.0,
        nonblocking: bool = False,
    ) -> None:
        self.fd = fd
        self.timeout = timeout
        self._closed = False

        if nonblocking:
            self.set_nonblocking()

    @classmethod
    def create(
        cls,
        timeout: float,
        udp: bool = False,
    ) -> "NativeStcpSocket":
        # AF_STCP exposes a BSD stream API for both carriers. Protocol 253
        # selects the TCP carrier and protocol 254 selects the UDP carrier.
        protocol = STCP_PROTOCOL_UDP if udp else STCP_PROTOCOL_TCP
        fd = libc.socket(AF_STCP, socket.SOCK_STREAM, protocol)
        if fd < 0:
            transport = "UDP" if udp else "TCP"
            error = ctypes.get_errno()
            raise OSError(
                error,
                f"socket({transport} carrier): {os.strerror(error)}",
            )

        # Keep the descriptor blocking for bind/listen or connect. The
        # current STCP connect path completes its handshake synchronously.
        return cls(fd, timeout, nonblocking=False)

    def set_nonblocking(self) -> None:
        flags = fcntl.fcntl(self.fd, fcntl.F_GETFL)
        if not flags & os.O_NONBLOCK:
            fcntl.fcntl(
                self.fd,
                fcntl.F_SETFL,
                flags | os.O_NONBLOCK,
            )

    def settimeout(self, timeout: float) -> None:
        self.timeout = timeout

    def _wait(self, events: int) -> None:
        # close() swaps fd to -1 before closing the original descriptor.  A
        # concurrent shutdown must become a normal EBADF path, not an uncaught
        # ValueError from select.poll.register().
        fd = self.fd
        if self._closed or fd < 0:
            raise OSError(errno.EBADF, "STCP socket is closed")

        poller = select.poll()
        try:
            poller.register(fd, events | select.POLLERR | select.POLLHUP)
        except ValueError as exc:
            raise OSError(errno.EBADF, "STCP socket closed during poll setup") from exc

        # Do not block for the complete socket timeout in one poll() call.
        # Linux does not guarantee that close() in another thread wakes a
        # concurrently blocked poll() for a custom protocol descriptor. Poll
        # in short slices so shutdown is observed deterministically.
        deadline = time.monotonic() + self.timeout
        ready = []
        while not ready:
            if self._closed or self.fd != fd or fd < 0:
                raise OSError(errno.EBADF, "STCP socket closed while waiting")

            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise TimeoutError(f"fd {fd} timed out")

            slice_ms = max(1, min(100, int(remaining * 1000)))
            ready = poller.poll(slice_ms)

        revents = ready[0][1]
        if revents & select.POLLNVAL:
            raise OSError(errno.EBADF, "invalid STCP fd")
        if revents & select.POLLERR:
            raise OSError(errno.EPROTO, "STCP poll reported POLLERR")
        if revents & select.POLLHUP:
            raise ConnectionError("STCP peer disconnected")
        if not (revents & events):
            raise OSError(errno.EIO, f"unexpected STCP poll events: 0x{revents:x}")

    def bind(self, address: tuple[str, int]) -> None:
        addr = _sockaddr(*address)
        if libc.bind(self.fd, ctypes.byref(addr), ctypes.sizeof(addr)) < 0:
            _raise_errno("bind")

    def listen(self, backlog: int) -> None:
        if libc.listen(self.fd, backlog) < 0:
            _raise_errno("listen")

        # accept() is driven through poll(), so the listener becomes
        # nonblocking only after bind/listen have completed.
        self.set_nonblocking()

    def accept(self) -> tuple["NativeStcpSocket", None]:
        while True:
            self._wait(select.POLLIN)
            fd = libc.accept(self.fd, None, None)
            if fd >= 0:
                return NativeStcpSocket(
                    fd,
                    self.timeout,
                    nonblocking=True,
                ), None
            error = ctypes.get_errno()
            if error in (errno.EAGAIN, errno.EWOULDBLOCK, errno.EINTR):
                continue
            _raise_errno("accept")

    def connect(self, address: tuple[str, int]) -> None:
        addr = _sockaddr(*address)

        # STCP currently performs connection setup and the crypto handshake
        # synchronously inside connect(). Using O_NONBLOCK here leaves the
        # socket in progress, but the current poll implementation does not
        # advertise connect completion like TCP does.
        result = libc.connect(
            self.fd,
            ctypes.byref(addr),
            ctypes.sizeof(addr),
        )

        if result < 0:
            error = ctypes.get_errno()
            raise OSError(
                error,
                f"connect: {os.strerror(error)}",
            )

        # All subsequent send/recv operations are poll driven.
        self.set_nonblocking()

    def send(self, data: object) -> int:
        view = memoryview(data).cast("B")
        if len(view) == 0:
            return 0
        if view.readonly:
            raise TypeError("send buffer must be writable to avoid a copy")
        array = (ctypes.c_ubyte * len(view)).from_buffer(view)

        while True:
            result = libc.send(
                self.fd,
                array,
                len(view),
                getattr(socket, "MSG_NOSIGNAL", 0),
            )
            if result >= 0:
                return int(result)
            error = ctypes.get_errno()
            if error in (errno.EAGAIN, errno.EWOULDBLOCK):
                self._wait(select.POLLOUT)
                continue
            if error == errno.EINTR:
                continue
            _raise_errno("send")

    def recv_into(self, buffer: object, nbytes: int = 0) -> int:
        view = memoryview(buffer).cast("B")
        count = len(view) if nbytes <= 0 else min(len(view), nbytes)
        if count == 0:
            return 0
        if view.readonly:
            raise TypeError("recv_into requires a writable buffer")
        array = (ctypes.c_ubyte * count).from_buffer(view)

        while True:
            result = libc.recv(self.fd, array, count, 0)
            if result >= 0:
                return int(result)
            error = ctypes.get_errno()
            if error in (errno.EAGAIN, errno.EWOULDBLOCK):
                self._wait(select.POLLIN)
                continue
            if error == errno.EINTR:
                continue
            _raise_errno("recv")

    def close(self) -> None:
        if self._closed:
            return
        self._closed = True
        fd, self.fd = self.fd, -1
        if libc.close(fd) < 0:
            error = ctypes.get_errno()
            if error != errno.EBADF:
                raise OSError(error, os.strerror(error))


class StopState:
    def __init__(self) -> None:
        self.event = threading.Event()

    def stop(self) -> None:
        self.event.set()

    def stopped(self) -> bool:
        return self.event.is_set()


def create_stcp_socket(
    timeout: float,
    udp: bool = False,
) -> NativeStcpSocket:
    return NativeStcpSocket.create(timeout, udp=udp)


def send_all(
    sock: NativeStcpSocket,
    data: bytes,
    stop: StopState,
) -> int:
    view = memoryview(data)
    total = 0

    while total < len(view) and not stop.stopped():
        sent = sock.send(view[total:])
        if sent == 0:
            raise ConnectionError("send returned zero")
        total += sent

    return total


def recv_exact_into(
    sock: NativeStcpSocket,
    output: bytearray,
    expected: int,
    stop: StopState,
) -> int:
    view = memoryview(output)
    total = 0

    while total < expected and not stop.stopped():
        received = sock.recv_into(view[total:expected], expected - total)
        if received == 0:
            raise ConnectionError(
                f"peer closed after {total}/{expected} bytes"
            )
        total += received

    if total != expected:
        raise InterruptedError(
            f"stopped after {total}/{expected} bytes"
        )

    return total


def recv_to_echo(
    sock: NativeStcpSocket,
    stop: StopState,
    buffer: bytearray,
) -> Optional[memoryview]:
    """Receive one echo request.

    A clean STCP CLOSE is returned as zero bytes (BSD EOF). Timeouts are still
    propagated as a defensive fallback for a crashed or ungraceful peer.
    """
    if stop.stopped():
        return None

    count = sock.recv_into(buffer)
    if count == 0:
        return None

    return memoryview(buffer)[:count]


def server_connection(
    conn: NativeStcpSocket,
    stop: StopState,
    stats: SharedStats,
    recv_buffer_size: int,
) -> None:
    buffer = bytearray(recv_buffer_size)
    idle_timeouts = 0
    # Normal churn exits immediately through protocol EOF. Keep a longer
    # fallback only for peers that disappear without transmitting CLOSE.
    max_idle_timeouts = 10

    try:
        conn.settimeout(1.0)

        while not stop.stopped():
            try:
                data = recv_to_echo(conn, stop, buffer)
            except TimeoutError:
                idle_timeouts += 1
                if idle_timeouts >= max_idle_timeouts:
                    # Defensive fallback for a crashed/ungraceful peer. Clean
                    # close(fd) now arrives as recv()==0 through STCP CLOSE.
                    break
                continue

            if data is None:
                break

            idle_timeouts = 0
            sent = 0
            while sent < len(data) and not stop.stopped():
                count = conn.send(data[sent:])
                if count == 0:
                    raise ConnectionError("server send returned zero")
                sent += count

    except (OSError, ConnectionError):
        if not stop.stopped():
            stats.add(server_errors=1)
    finally:
        try:
            conn.close()
        except OSError:
            pass


def server_main(
    host: str,
    port: int,
    backlog: int,
    stop: StopState,
    stats: SharedStats,
    recv_buffer_size: int,
    ready: threading.Event,
    udp: bool,
) -> None:
    listener: Optional[NativeStcpSocket] = None
    handlers: set[threading.Thread] = set()
    handlers_lock = threading.Lock()

    def reap_handlers() -> None:
        with handlers_lock:
            finished = [thread for thread in handlers if not thread.is_alive()]
            for thread in finished:
                handlers.discard(thread)

    try:
        listener = create_stcp_socket(timeout=1.0, udp=udp)
        listener.bind((host, port))
        listener.listen(backlog)
        ready.set()

        while not stop.stopped():
            reap_handlers()

            # Bound accepted descriptors even if a protocol regression makes
            # a handler slow to notice an idle/closed peer.
            with handlers_lock:
                active_handlers = len(handlers)
            if active_handlers >= backlog:
                time.sleep(0.01)
                continue

            try:
                conn, _ = listener.accept()
            except socket.timeout:
                continue
            except OSError as exc:
                if stop.stopped():
                    break
                stats.add(server_errors=1)
                print(f"server accept error: {exc!r}", file=sys.stderr, flush=True)
                if exc.errno in (errno.EMFILE, errno.ENFILE):
                    # Give completed handlers time to close and be reaped.
                    time.sleep(0.1)
                continue

            stats.add(server_accepts=1)
            thread = threading.Thread(
                target=server_connection,
                args=(
                    conn,
                    stop,
                    stats,
                    recv_buffer_size,
                ),
                daemon=True,
            )
            with handlers_lock:
                handlers.add(thread)
            thread.start()
            reap_handlers()

    except OSError as exc:
        print(f"server setup failed: {exc}", file=sys.stderr)
        stop.stop()
        ready.set()
    finally:
        if listener is not None:
            try:
                listener.close()
            except OSError:
                pass

        with handlers_lock:
            remaining_handlers = list(handlers)

        for thread in remaining_handlers:
            thread.join(timeout=2.0)


def payload_for(worker_id: int, payload_size: int) -> bytearray:
    header = worker_id.to_bytes(4, "big", signed=False)
    pattern = bytes(((worker_id * 31 + index) & 0xFF) for index in range(256))

    output = bytearray(payload_size)
    output[: min(4, payload_size)] = header[: min(4, payload_size)]

    position = 4
    while position < payload_size:
        count = min(len(pattern), payload_size - position)
        output[position:position + count] = pattern[:count]
        position += count

    return output


def run_exchange(
    sock: NativeStcpSocket,
    payload: bytearray,
    reply_buffer: bytearray,
    stop: StopState,
    stats: SharedStats,
    verify: bool,
) -> bool:
    started = time.perf_counter_ns()

    try:
        sent = send_all(sock, payload, stop)
        stats.add(sends_ok=1, bytes_sent=sent)
    except (OSError, ConnectionError, InterruptedError):
        if not stop.stopped():
            stats.add(sends_failed=1)
        return False

    try:
        received = recv_exact_into(
            sock,
            reply_buffer,
            len(payload),
            stop,
        )
        stats.add(recvs_ok=1, bytes_received=received)
    except (OSError, ConnectionError, InterruptedError):
        if not stop.stopped():
            stats.add(recvs_failed=1)
        return False

    elapsed_ms = (time.perf_counter_ns() - started) / 1_000_000.0
    stats.add_latency(elapsed_ms)

    if verify and reply_buffer != payload:
        stats.add(verify_failures=1)
        return False

    return True


def run_pipelined_batch(
    sock: NativeStcpSocket,
    payload: bytearray,
    reply_buffer: bytearray,
    batch_size: int,
    stop: StopState,
    stats: SharedStats,
    verify: bool,
) -> bool:
    payload_len = len(payload)
    expected_total = payload_len * batch_size
    started = time.perf_counter_ns()

    try:
        for _ in range(batch_size):
            sent = send_all(sock, payload, stop)
            if sent != payload_len:
                raise ConnectionError(
                    f"partial send {sent}/{payload_len}"
                )

        stats.add(
            sends_ok=batch_size,
            bytes_sent=expected_total,
        )
    except (OSError, ConnectionError, InterruptedError):
        if not stop.stopped():
            stats.add(sends_failed=1)
        return False

    received_total = 0
    payload_view = memoryview(payload)

    try:
        while received_total < expected_total and not stop.stopped():
            request = min(
                len(reply_buffer),
                expected_total - received_total,
            )

            received = sock.recv_into(
                memoryview(reply_buffer)[:request]
            )

            if received == 0:
                raise ConnectionError(
                    f"peer closed after {received_total}/{expected_total}"
                )

            if verify:
                chunk = memoryview(reply_buffer)[:received]
                offset = received_total % payload_len
                position = 0

                while position < received:
                    count = min(
                        received - position,
                        payload_len - offset,
                    )

                    if chunk[position:position + count] != \
                       payload_view[offset:offset + count]:
                        stats.add(verify_failures=1)
                        return False

                    position += count
                    offset = (offset + count) % payload_len

            received_total += received

        if received_total != expected_total:
            raise InterruptedError(
                f"stopped after {received_total}/{expected_total}"
            )

        stats.add(
            recvs_ok=batch_size,
            bytes_received=received_total,
        )
    except (OSError, ConnectionError, InterruptedError):
        if not stop.stopped():
            stats.add(recvs_failed=1)
        return False

    elapsed_ms = (time.perf_counter_ns() - started) / 1_000_000.0
    stats.add_latency(elapsed_ms / batch_size)
    return True


def persistent_client(
    worker_id: int,
    host: str,
    port: int,
    payload_size: int,
    timeout: float,
    stop: StopState,
    stats: SharedStats,
    verify: bool,
    pipeline: int,
    udp: bool,
) -> None:
    payload = payload_for(worker_id, payload_size)
    reply_buffer = bytearray(payload_size)
    sock: Optional[NativeStcpSocket] = None

    try:
        sock = create_stcp_socket(timeout, udp=udp)
        sock.connect((host, port))
        stats.add(connections_ok=1)

        while not stop.stopped():
            if not run_pipelined_batch(
                sock,
                payload,
                reply_buffer,
                pipeline,
                stop,
                stats,
                verify,
            ):
                break

    except OSError as exc:
        if not stop.stopped():
            stats.add(connections_failed=1)
            print(f"client {worker_id} connect/runtime error: {exc!r}", file=sys.stderr, flush=True)
    finally:
        if sock is not None:
            try:
                sock.close()
            except OSError:
                pass


def churn_client(
    worker_id: int,
    host: str,
    port: int,
    payload_size: int,
    timeout: float,
    stop: StopState,
    stats: SharedStats,
    verify: bool,
    udp: bool,
) -> None:
    payload = payload_for(worker_id, payload_size)
    reply_buffer = bytearray(payload_size)

    while not stop.stopped():
        sock: Optional[NativeStcpSocket] = None

        try:
            sock = create_stcp_socket(timeout, udp=udp)
            sock.connect((host, port))
            stats.add(connections_ok=1)

            run_exchange(
                sock,
                payload,
                reply_buffer,
                stop,
                stats,
                verify,
            )

        except OSError:
            if not stop.stopped():
                stats.add(connections_failed=1)
        finally:
            if sock is not None:
                try:
                    sock.close()
                except OSError:
                    pass


def mixed_client(
    worker_id: int,
    host: str,
    port: int,
    payload_size: int,
    timeout: float,
    stop: StopState,
    stats: SharedStats,
    verify: bool,
    reconnect_every: int,
    udp: bool,
) -> None:
    payload = payload_for(worker_id, payload_size)
    reply_buffer = bytearray(payload_size)

    while not stop.stopped():
        sock: Optional[NativeStcpSocket] = None

        try:
            sock = create_stcp_socket(timeout, udp=udp)
            sock.connect((host, port))
            stats.add(connections_ok=1)

            for _ in range(reconnect_every):
                if stop.stopped():
                    break

                if not run_exchange(
                    sock,
                    payload,
                    reply_buffer,
                    stop,
                    stats,
                    verify,
                ):
                    break

        except OSError:
            if not stop.stopped():
                stats.add(connections_failed=1)
        finally:
            if sock is not None:
                try:
                    sock.close()
                except OSError:
                    pass


def percentile(values: list[float], percent: float) -> float:
    if not values:
        return 0.0

    ordered = sorted(values)
    index = math.ceil((percent / 100.0) * len(ordered)) - 1
    index = max(0, min(index, len(ordered) - 1))
    return ordered[index]


def process_cpu_seconds() -> float:
    usage = os.times()
    return usage.user + usage.system


def process_rss_mib() -> float:
    try:
        status = Path("/proc/self/status").read_text()
        for line in status.splitlines():
            if line.startswith("VmRSS:"):
                kib = int(line.split()[1])
                return kib / 1024.0
    except (OSError, ValueError, IndexError):
        pass

    return 0.0


def fmt_rate(byte_count: int, seconds: float) -> str:
    if seconds <= 0:
        return "0.00 MiB/s"
    return f"{byte_count / seconds / (1024 * 1024):.2f} MiB/s"


def report_line(
    elapsed: float,
    interval: float,
    current: Counters,
    delta: Counters,
    interval_latencies: list[float],
    cpu_percent: float,
    rss_mib: float,
) -> str:
    p50 = percentile(interval_latencies, 50)
    p95 = percentile(interval_latencies, 95)
    p99 = percentile(interval_latencies, 99)

    return (
        f"[{elapsed:7.1f}s] "
        f"TX {fmt_rate(delta.bytes_sent, interval):>12}  "
        f"RX {fmt_rate(delta.bytes_received, interval):>12}  "
        f"ops {delta.recvs_ok / interval:8.1f}/s  "
        f"conn +{delta.connections_ok}/-{delta.connections_failed}  "
        f"errors s={delta.sends_failed} r={delta.recvs_failed} "
        f"v={delta.verify_failures} srv={delta.server_errors}  "
        f"RTT p50={p50:7.2f} p95={p95:7.2f} p99={p99:7.2f} ms  "
        f"CPU={cpu_percent:6.1f}% RSS={rss_mib:7.1f} MiB  "
        f"total={current.bytes_received / (1024 * 1024):.1f} MiB"
    )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="STCP kernel-module stress and throughput test"
    )
    parser.add_argument(
        "--mode",
        choices=("throughput", "churn", "mixed"),
        default="throughput",
    )
    parser.add_argument(
        "--udp",
        action="store_true",
        help=(
            "use STCP protocol 254 (UDP carrier); default is protocol 253 "
            "(TCP carrier)"
        ),
    )
    parser.add_argument("--host", default=DEFAULT_HOST)
    parser.add_argument("--port", type=int, default=DEFAULT_PORT)
    parser.add_argument("--clients", type=int, default=16)
    parser.add_argument("--duration", type=float, default=180.0)
    parser.add_argument("--report-every", "--report-interval", dest="report_every", type=float, default=5.0)
    parser.add_argument(
        "--payload",
        type=int,
        default=256 * 1024,
        help="payload bytes per request",
    )
    parser.add_argument("--backlog", type=int, default=256)
    parser.add_argument("--timeout", type=float, default=10.0)
    parser.add_argument(
        "--reconnect-every",
        type=int,
        default=32,
        help="mixed mode exchanges per connection",
    )
    parser.add_argument(
        "--no-verify",
        action="store_true",
        help="skip echoed payload verification",
    )
    parser.add_argument(
        "--json",
        default="stcp-stress-result.json",
        help="summary output path",
    )
    parser.add_argument(
        "--latency-samples",
        type=int,
        default=100_000,
    )
    parser.add_argument(
        "--pipeline",
        type=int,
        default=1,
        help="outstanding payloads per throughput connection",
    )
    return parser.parse_args()


def validate_args(args: argparse.Namespace) -> None:
    if args.clients <= 0:
        raise ValueError("--clients must be positive")
    if args.duration <= 0:
        raise ValueError("--duration must be positive")
    if args.report_every <= 0:
        raise ValueError("--report-every must be positive")
    if args.payload <= 0:
        raise ValueError("--payload must be positive")
    if args.backlog <= 0:
        raise ValueError("--backlog must be positive")
    if args.timeout <= 0:
        raise ValueError("--timeout must be positive")
    if args.pipeline <= 0:
        raise ValueError("--pipeline must be positive")


def main() -> int:
    args = parse_args()

    try:
        validate_args(args)
    except ValueError as exc:
        print(f"argument error: {exc}", file=sys.stderr)
        return 2

    stop = StopState()
    stats = SharedStats(args.latency_samples)
    ready = threading.Event()

    def handle_signal(_signum: int, _frame: object) -> None:
        stop.stop()

    signal.signal(signal.SIGINT, handle_signal)
    signal.signal(signal.SIGTERM, handle_signal)

    # Only active server connections own this buffer.  Keep it large enough
    # for efficient reads but do not reserve 1 MiB for every churned thread.
    recv_buffer_size = min(
        max(args.payload, 64 * 1024),
        256 * 1024,
    )

    server = threading.Thread(
        target=server_main,
        args=(
            args.host,
            args.port,
            args.backlog,
            stop,
            stats,
            recv_buffer_size,
            ready,
            args.udp,
        ),
        name="stcp-server",
        daemon=True,
    )
    server.start()

    if not ready.wait(timeout=args.timeout):
        print("server did not become ready", file=sys.stderr)
        stop.stop()
        return 1

    if stop.stopped():
        print("server setup failed", file=sys.stderr)
        return 1

    client_target = {
        "throughput": persistent_client,
        "churn": churn_client,
        "mixed": mixed_client,
    }[args.mode]

    clients: list[threading.Thread] = []

    for worker_id in range(args.clients):
        common = (
            worker_id,
            args.host,
            args.port,
            args.payload,
            args.timeout,
            stop,
            stats,
            not args.no_verify,
        )

        if args.mode == "mixed":
            thread_args = common + (args.reconnect_every, args.udp)
        elif args.mode == "throughput":
            thread_args = common + (args.pipeline, args.udp)
        else:
            thread_args = common + (args.udp,)

        thread = threading.Thread(
            target=client_target,
            args=thread_args,
            name=f"stcp-client-{worker_id}",
            daemon=True,
        )
        clients.append(thread)
        thread.start()

    print("=== STCP Python stress test ===")
    print(f"transport:        {'UDP' if args.udp else 'TCP'}")
    print(f"protocol:         {STCP_PROTOCOL_UDP if args.udp else STCP_PROTOCOL_TCP}")
    print(f"mode:             {args.mode}")
    print(f"clients:          {args.clients}")
    print(f"payload:          {args.payload} bytes")
    print(f"duration:         {args.duration:.1f} s")
    print(f"report interval:  {args.report_every:.1f} s")
    print(f"verify payload:   {not args.no_verify}")
    if args.mode == "throughput":
        print(f"pipeline depth:   {args.pipeline}")
    print()

    started = time.monotonic()
    previous_time = started
    previous_counters = Counters()
    previous_latency_count = 0
    previous_cpu = process_cpu_seconds()

    try:
        while not stop.stopped():
            target_time = min(
                started + args.duration,
                previous_time + args.report_every,
            )
            sleep_time = target_time - time.monotonic()

            if sleep_time > 0:
                stop.event.wait(sleep_time)

            now = time.monotonic()
            elapsed = now - started

            current, latencies = stats.snapshot()
            delta = current.delta(previous_counters)
            interval = max(now - previous_time, 1e-9)

            interval_latencies = latencies[previous_latency_count:]
            previous_latency_count = len(latencies)

            cpu_now = process_cpu_seconds()
            cpu_percent = (
                (cpu_now - previous_cpu) / interval * 100.0
            )
            previous_cpu = cpu_now

            print(
                report_line(
                    elapsed,
                    interval,
                    current,
                    delta,
                    interval_latencies,
                    cpu_percent,
                    process_rss_mib(),
                ),
                flush=True,
            )

            previous_time = now
            previous_counters = current

            if elapsed >= args.duration:
                stop.stop()
                break

    finally:
        stop.stop()

        # Use one shared shutdown deadline. Without this, N client threads
        # can each consume the full timeout and a 20-second run may take
        # roughly 40 seconds to finish.
        shutdown_deadline = time.monotonic() + 2.0

        for thread in clients:
            remaining = max(
                0.0,
                shutdown_deadline - time.monotonic(),
            )
            thread.join(timeout=remaining)

        remaining = max(
            0.0,
            shutdown_deadline - time.monotonic(),
        )
        server.join(timeout=remaining)

    finished = time.monotonic()
    elapsed = max(finished - started, 1e-9)
    counters, latencies = stats.snapshot()

    summary = {
        "transport": "udp" if args.udp else "tcp",
        "protocol": STCP_PROTOCOL_UDP if args.udp else STCP_PROTOCOL_TCP,
        "mode": args.mode,
        "host": args.host,
        "port": args.port,
        "clients": args.clients,
        "payload_bytes": args.payload,
        "duration_seconds": elapsed,
        "report_interval_seconds": args.report_every,
        "verify_payload": not args.no_verify,
        "connections_ok": counters.connections_ok,
        "connections_failed": counters.connections_failed,
        "sends_ok": counters.sends_ok,
        "sends_failed": counters.sends_failed,
        "recvs_ok": counters.recvs_ok,
        "recvs_failed": counters.recvs_failed,
        "verify_failures": counters.verify_failures,
        "server_accepts": counters.server_accepts,
        "server_errors": counters.server_errors,
        "bytes_sent": counters.bytes_sent,
        "bytes_received": counters.bytes_received,
        "tx_mib_per_second": (
            counters.bytes_sent / elapsed / (1024 * 1024)
        ),
        "rx_mib_per_second": (
            counters.bytes_received / elapsed / (1024 * 1024)
        ),
        "operations_per_second": counters.recvs_ok / elapsed,
        "latency_ms": {
            "samples": len(latencies),
            "average": statistics.fmean(latencies) if latencies else 0.0,
            "p50": percentile(latencies, 50),
            "p95": percentile(latencies, 95),
            "p99": percentile(latencies, 99),
            "maximum": max(latencies) if latencies else 0.0,
        },
    }

    output_path = Path(args.json)
    output_path.write_text(
        json.dumps(summary, indent=2, sort_keys=True) + "\n"
    )

    print()
    print("=== Final result ===")
    print(f"elapsed:             {elapsed:.3f} s")
    print(f"connections ok:      {counters.connections_ok}")
    print(f"connections failed:  {counters.connections_failed}")
    print(f"sends ok/failed:     {counters.sends_ok}/{counters.sends_failed}")
    print(f"recvs ok/failed:     {counters.recvs_ok}/{counters.recvs_failed}")
    print(f"verify failures:     {counters.verify_failures}")
    print(f"server errors:       {counters.server_errors}")
    print(f"bytes sent:          {counters.bytes_sent}")
    print(f"bytes received:      {counters.bytes_received}")
    print(f"TX throughput:       {summary['tx_mib_per_second']:.2f} MiB/s")
    print(f"RX throughput:       {summary['rx_mib_per_second']:.2f} MiB/s")
    print(f"operations:          {summary['operations_per_second']:.1f}/s")
    print(
        "RTT latency:         "
        f"avg={summary['latency_ms']['average']:.2f} "
        f"p50={summary['latency_ms']['p50']:.2f} "
        f"p95={summary['latency_ms']['p95']:.2f} "
        f"p99={summary['latency_ms']['p99']:.2f} "
        f"max={summary['latency_ms']['maximum']:.2f} ms"
    )
    print(f"JSON result:         {output_path}")

    failed = any(
        (
            counters.connections_failed,
            counters.sends_failed,
            counters.recvs_failed,
            counters.verify_failures,
            counters.server_errors,
        )
    )

    return 1 if failed else 0


if __name__ == "__main__":
    raise SystemExit(main())
