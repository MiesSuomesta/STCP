#!/usr/bin/env python3
import argparse
import asyncio
import os
import signal
import statistics
import struct
import sys
import time
from dataclasses import dataclass, field
from typing import List, Optional

def now_ns() -> int:
    return time.perf_counter_ns()

def fmt_num(n: float) -> str:
    if n >= 1e9:  return f"{n/1e9:.2f}G"
    if n >= 1e6:  return f"{n/1e6:.2f}M"
    if n >= 1e3:  return f"{n/1e3:.2f}k"
    return f"{n:.0f}"

def percentile(values: List[float], p: float) -> float:
    if not values:
        return float("nan")
    vs = sorted(values)
    k = (len(vs) - 1) * p
    f = int(k)
    c = min(f + 1, len(vs) - 1)
    if f == c:
        return vs[f]
    return vs[f] + (vs[c] - vs[f]) * (k - f)

@dataclass
class Stats:
    connects: int = 0
    connect_errors: int = 0
    ops: int = 0
    op_errors: int = 0
    bytes_sent: int = 0
    bytes_recv: int = 0
    lat_ms: List[float] = field(default_factory=list)
    first_error: Optional[str] = None

    def note_error(self, msg: str, connect: bool = False) -> None:
        if self.first_error is None:
            self.first_error = msg
        if connect:
            self.connect_errors += 1
        else:
            self.op_errors += 1

def summarize(stats: Stats, started_ns: int) -> str:
    dur_s = (now_ns() - started_ns) / 1e9
    rps = stats.ops / dur_s if dur_s > 0 else 0.0
    bps = (stats.bytes_sent + stats.bytes_recv) / dur_s if dur_s > 0 else 0.0

    lat = stats.lat_ms
    lat_avg = statistics.mean(lat) if lat else float("nan")
    p50 = percentile(lat, 0.50) if lat else float("nan")
    p95 = percentile(lat, 0.95) if lat else float("nan")
    p99 = percentile(lat, 0.99) if lat else float("nan")

    return (
        f"duration={dur_s:.2f}s  "
        f"connects={stats.connects}  connect_err={stats.connect_errors}  "
        f"ops={stats.ops}  op_err={stats.op_errors}  "
        f"rps={rps:.1f}  throughput={fmt_num(bps)}/s  "
        f"lat_ms(avg/p50/p95/p99)={lat_avg:.3f}/{p50:.3f}/{p95:.3f}/{p99:.3f}"
    )

async def read_exact(reader: asyncio.StreamReader, n: int, timeout: float) -> bytes:
    buf = b""
    while len(buf) < n:
        chunk = await asyncio.wait_for(reader.read(n - len(buf)), timeout=timeout)
        if not chunk:
            raise ConnectionError("EOF")
        buf += chunk
    return buf

async def rpc(writer: asyncio.StreamWriter, reader: asyncio.StreamReader, payload: bytes, timeout: float) -> bytes:
    # App-level framing: 4B BE length + payload
    writer.write(struct.pack("!I", len(payload)) + payload)
    await asyncio.wait_for(writer.drain(), timeout=timeout)
    hdr = await read_exact(reader, 4, timeout)
    (ln,) = struct.unpack("!I", hdr)
    if ln > 32 * 1024 * 1024:
        raise ValueError(f"response too big: {ln}")
    return await read_exact(reader, ln, timeout)

async def churn_worker(wid: int, host: str, port: int, duration_s: float,
                       msg_size: int, timeout: float, stats: Stats,
                       stop_event: asyncio.Event) -> None:
    end_t = time.monotonic() + duration_s
    while time.monotonic() < end_t and not stop_event.is_set():
        try:
            stats.connects += 1
            reader, writer = await asyncio.wait_for(asyncio.open_connection(host, port), timeout=timeout)
        except Exception as e:
            stats.note_error(f"[worker {wid}] connect failed: {e!r}", connect=True)
            continue

        try:
            payload = os.urandom(msg_size)
            t0 = now_ns()
            resp = await rpc(writer, reader, payload, timeout)
            dt_ms = (now_ns() - t0) / 1e6

            stats.bytes_sent += (4 + len(payload))
            stats.bytes_recv += (4 + len(resp))
            stats.lat_ms.append(dt_ms)
            stats.ops += 1

            if resp != payload:
                raise AssertionError(f"echo verify failed: resp_len={len(resp)}")

        except Exception as e:
            stats.note_error(f"[worker {wid}] op failed: {e!r}", connect=False)
        finally:
            try:
                writer.close()
                await writer.wait_closed()
            except Exception:
                pass

async def steady_worker(wid: int, host: str, port: int, duration_s: float,
                        msg_size: int, messages_per_conn: int, timeout: float,
                        stats: Stats, stop_event: asyncio.Event) -> None:
    try:
        stats.connects += 1
        reader, writer = await asyncio.wait_for(asyncio.open_connection(host, port), timeout=timeout)
    except Exception as e:
        stats.note_error(f"[worker {wid}] connect failed: {e!r}", connect=True)
        return

    end_t = time.monotonic() + duration_s
    try:
        while time.monotonic() < end_t and not stop_event.is_set():
            for _ in range(messages_per_conn):
                if time.monotonic() >= end_t or stop_event.is_set():
                    break
                payload = os.urandom(msg_size)
                t0 = now_ns()
                resp = await rpc(writer, reader, payload, timeout)
                dt_ms = (now_ns() - t0) / 1e6

                stats.bytes_sent += (4 + len(payload))
                stats.bytes_recv += (4 + len(resp))
                stats.lat_ms.append(dt_ms)
                stats.ops += 1

                if resp != payload:
                    raise AssertionError(f"echo verify failed: resp_len={len(resp)}")
    except Exception as e:
        stats.note_error(f"[worker {wid}] op failed: {e!r}", connect=False)
    finally:
        try:
            writer.close()
            await writer.wait_closed()
        except Exception:
            pass

async def reporter(stats: Stats, every_s: float, stop_event: asyncio.Event) -> None:
    last_ops = 0
    last_bytes = 0
    last_t = time.monotonic()
    while not stop_event.is_set():
        await asyncio.sleep(every_s)
        t = time.monotonic()
        dt = t - last_t
        ops = stats.ops - last_ops
        byt = (stats.bytes_sent + stats.bytes_recv) - last_bytes

        last_ops = stats.ops
        last_bytes = stats.bytes_sent + stats.bytes_recv
        last_t = t

        rps = ops / dt if dt > 0 else 0.0
        bps = byt / dt if dt > 0 else 0.0
        print(f"[{time.strftime('%H:%M:%S')}] rps={rps:.1f} throughput={fmt_num(bps)}/s  total_ops={stats.ops}  err={stats.op_errors}+{stats.connect_errors}")

async def run(args) -> int:
    stats = Stats()
    started_ns = now_ns()
    stop_event = asyncio.Event()

    def on_stop():
        stop_event.set()

    loop = asyncio.get_running_loop()
    for sig in (signal.SIGINT, signal.SIGTERM):
        try:
            loop.add_signal_handler(sig, on_stop)
        except NotImplementedError:
            pass

    tasks = []
    if args.report_every > 0:
        tasks.append(asyncio.create_task(reporter(stats, args.report_every, stop_event)))

    for i in range(args.clients):
        if args.mode == "churn":
            tasks.append(asyncio.create_task(
                churn_worker(i, args.host, args.port, args.duration, args.msg_size, args.timeout, stats, stop_event)
            ))
        else:
            tasks.append(asyncio.create_task(
                steady_worker(i, args.host, args.port, args.duration, args.msg_size, args.messages_per_conn, args.timeout, stats, stop_event)
            ))

    try:
        await asyncio.wait_for(asyncio.gather(*tasks), timeout=args.duration + 10.0)
    except asyncio.TimeoutError:
        stop_event.set()

    print("\n=== SUMMARY ===")
    print(summarize(stats, started_ns))
    if stats.first_error:
        print(f"first_error={stats.first_error}")
        return 2
    return 0

def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="STCP framed stress test client (asyncio).")
    p.add_argument("--host", default="127.0.0.1")
    p.add_argument("--port", type=int, required=True)
    p.add_argument("--mode", choices=["churn", "steady"], default="steady")
    p.add_argument("--clients", type=int, default=50)
    p.add_argument("--duration", type=float, default=30.0)
    p.add_argument("--msg-size", type=int, default=256)
    p.add_argument("--messages-per-conn", type=int, default=200)
    p.add_argument("--timeout", type=float, default=5.0)
    p.add_argument("--report-every", type=float, default=1.0)
    return p.parse_args()

if __name__ == "__main__":
    args = parse_args()
    try:
        sys.exit(asyncio.run(run(args)))
    except KeyboardInterrupt:
        sys.exit(130)

