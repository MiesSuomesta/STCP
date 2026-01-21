#!/usr/bin/env python3
import asyncio
import argparse
import socket
import time
import json
import errno
import struct
from dataclasses import dataclass
from typing import Optional

IPPROTO_STCP = 253
MAX_FRAME = 32 * 1024 * 1024


@dataclass
class Stats:
    connects: int = 0
    sends: int = 0
    recvs: int = 0
    errors: int = 0
    connect_errors: int = 0
    send_errors: int = 0
    recv_errors: int = 0
    last_error: Optional[str] = None

    def note_error(self, msg: str, connect=False, send=False, recv=False):
        self.errors += 1
        self.last_error = msg
        if connect:
            self.connect_errors += 1
        if send:
            self.send_errors += 1
        if recv:
            self.recv_errors += 1


def normalize_ipv4(host: str) -> str:
    socket.inet_aton(host)
    return host


def make_sock(proto: int) -> socket.socket:
    if proto == IPPROTO_STCP:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, IPPROTO_STCP)
    else:
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.setblocking(False)
    return s


async def sock_connect_no_gai(sock: socket.socket, addr, timeout: float) -> None:
    loop = asyncio.get_running_loop()

    err = sock.connect_ex(addr)
    if err == 0:
        return
    if err not in (errno.EINPROGRESS, errno.EALREADY, errno.EWOULDBLOCK):
        raise OSError(err, errno.errorcode.get(err, "connect_ex"))

    fut = loop.create_future()

    def writable():
        try:
            e = sock.getsockopt(socket.SOL_SOCKET, socket.SO_ERROR)
            if e == 0:
                if not fut.done():
                    fut.set_result(None)
            else:
                if not fut.done():
                    fut.set_exception(OSError(e, errno.errorcode.get(e, "SO_ERROR")))
        finally:
            loop.remove_writer(sock.fileno())

    loop.add_writer(sock.fileno(), writable)
    try:
        await asyncio.wait_for(fut, timeout)
    finally:
        loop.remove_writer(sock.fileno())


async def sock_recv_exact(loop: asyncio.AbstractEventLoop, sock: socket.socket, n: int, timeout: float) -> bytes:
    buf = bytearray()
    while len(buf) < n:
        chunk = await asyncio.wait_for(loop.sock_recv(sock, n - len(buf)), timeout)
        if not chunk:
            raise ConnectionError("peer closed")
        buf += chunk
    return bytes(buf)


async def open_sock_connected(host, port, proto, timeout):
    ip = normalize_ipv4(host)
    sock = make_sock(proto)
    await sock_connect_no_gai(sock, (ip, port), timeout)
    return sock


def frame_payload(payload: bytes) -> bytes:
    ln = len(payload)
    if ln > MAX_FRAME:
        raise ValueError(f"payload too big: {ln}")
    return struct.pack("!I", ln) + payload


async def recv_frame(loop: asyncio.AbstractEventLoop, sock: socket.socket, timeout: float) -> bytes:
    hdr = await sock_recv_exact(loop, sock, 4, timeout)
    (ln,) = struct.unpack("!I", hdr)
    if ln > MAX_FRAME:
        raise ValueError(f"frame too big: {ln}")
    data = await sock_recv_exact(loop, sock, ln, timeout)
    return data


async def steady_worker(wid, host, port, proto, payload, timeout, framed: bool, stats: Stats):
    loop = asyncio.get_running_loop()
    try:
        stats.connects += 1
        sock = await open_sock_connected(host, port, proto, timeout)
    except Exception as e:
        stats.note_error(f"[steady {wid}] connect failed: {e!r}", connect=True)
        print(f"[STEADY CONNECT FAIL] {e!r}", flush=True)
        return

    try:
        if framed:
            out = frame_payload(payload)
            while True:
                stats.sends += 1
                await asyncio.wait_for(loop.sock_sendall(sock, out), timeout)
                _ = await recv_frame(loop, sock, timeout)
                stats.recvs += 1
        else:
            while True:
                stats.sends += 1
                await asyncio.wait_for(loop.sock_sendall(sock, payload), timeout)
                await sock_recv_exact(loop, sock, len(payload), timeout)
                stats.recvs += 1
    except asyncio.CancelledError:
        pass
    except Exception as e:
        stats.note_error(f"[steady {wid}] io failed: {e!r}", send=True, recv=True)
        print(f"[STEADY IO FAIL] {e!r}", flush=True)
    finally:
        sock.close()


async def churn_worker(wid, host, port, proto, payload, timeout, messages_per_conn, framed: bool, stats: Stats):
    loop = asyncio.get_running_loop()
    out = frame_payload(payload) if framed else payload

    while True:
        try:
            stats.connects += 1
            sock = await open_sock_connected(host, port, proto, timeout)
        except Exception as e:
            stats.note_error(f"[churn {wid}] connect failed: {e!r}", connect=True)
            print(f"[CHURN CONNECT FAIL] {e!r}", flush=True)
            await asyncio.sleep(0.01)
            continue

        try:
            for _ in range(messages_per_conn):
                stats.sends += 1
                await asyncio.wait_for(loop.sock_sendall(sock, out), timeout)
                if framed:
                    _ = await recv_frame(loop, sock, timeout)
                else:
                    await sock_recv_exact(loop, sock, len(payload), timeout)
                stats.recvs += 1
        except asyncio.CancelledError:
            sock.close()
            raise
        except Exception as e:
            stats.note_error(f"[churn {wid}] io failed: {e!r}", send=True, recv=True)
            print(f"[CHURN IO FAIL] {e!r}", flush=True)
        finally:
            sock.close()


async def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--host", required=True)
    ap.add_argument("--port", type=int, required=True)
    ap.add_argument("--proto", type=int, default=6)
    ap.add_argument("--mode", choices=["steady", "churn"], required=True)
    ap.add_argument("--clients", type=int, default=1)
    ap.add_argument("--duration", type=int, default=30)
    ap.add_argument("--msg-size", type=int, default=256)
    ap.add_argument("--messages-per-conn", type=int, default=1)
    ap.add_argument("--timeout", type=float, default=10.0)
    ap.add_argument("--report-every", type=int, default=5)
    ap.add_argument("--summary-json", action="store_true")
    ap.add_argument("--summary-json-file")

    framing = ap.add_mutually_exclusive_group()
    framing.add_argument("--framed", action="store_true", help="Use 4B length + payload framing (default)")
    framing.add_argument("--raw", action="store_true", help="Send/recv raw bytes only (no framing)")

    args = ap.parse_args()

    try:
        normalize_ipv4(args.host)
    except Exception:
        raise SystemExit("ERROR: --host must be numeric IPv4 (e.g. 192.168.1.21)")

    framed = not args.raw
    payload = b"x" * args.msg_size
    stats = Stats()

    tasks = []
    for i in range(args.clients):
        if args.mode == "steady":
            tasks.append(asyncio.create_task(
                steady_worker(i, args.host, args.port, args.proto, payload, args.timeout, framed, stats)
            ))
        else:
            tasks.append(asyncio.create_task(
                churn_worker(i, args.host, args.port, args.proto, payload, args.timeout, args.messages_per_conn, framed, stats)
            ))

    start = time.time()
    last_sends = 0

    try:
        while time.time() - start < args.duration:
            await asyncio.sleep(args.report_every)
            sends_now = stats.sends - last_sends
            last_sends = stats.sends
            print(
                f"rps={sends_now/args.report_every:.1f} "
                f"connects={stats.connects} sends={stats.sends} "
                f"recvs={stats.recvs} errors={stats.errors} "
                f"(connect={stats.connect_errors} send={stats.send_errors} recv={stats.recv_errors})",
                flush=True,
            )
    finally:
        for t in tasks:
            t.cancel()
        await asyncio.gather(*tasks, return_exceptions=True)

    if args.summary_json and args.summary_json_file:
        with open(args.summary_json_file, "w") as f:
            json.dump(stats.__dict__, f, indent=2)


if __name__ == "__main__":
    asyncio.run(main())
