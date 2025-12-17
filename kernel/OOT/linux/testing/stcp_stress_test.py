#!/usr/bin/env python3
import argparse
import asyncio
import os
import random
import signal
import statistics
import struct
import sys
import time
from dataclasses import dataclass, field
from typing import List, Optional, Tuple

# --------- Helpers ---------

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
    values_sorted = sorted(values)
    k = (len(values_sorted) - 1) * p
    f = int(k)
    c = min(f + 1, len(values_sorted) - 1)
    if f == c:
        return values_sorted[f]
    return values_sorted[f] + (values_sorted[c] - values_sorted[f]) * (k - f)

# --------- Protocol adapters ---------
# Oletus: userspace puhuu plaintextia STCP-socketille, ja serveri vastaa jollain ACK/echo.
# Tämä on tarkoituksella "agnostinen": voit valita verify-moodin.
#
# verify=none : ei tarkastusta
# verify=prefix : odottaa että vastaus alkaa samalla prefixillä kuin lähetetty
# verify=echo : odottaa että vastaus == lähetetty (echo-server)

async def read_some(reader: asyncio.StreamReader, min_bytes: int, timeout: float) -> bytes:
    # Lue ainakin min_bytes, mutta palauta heti kun saat jotain jos min_bytes==1
    data = b""
    deadline = time.monotonic() + timeout
    while len(data) < min_bytes:
        remain = deadline - time.monotonic()
        if remain <= 0:
            raise asyncio.TimeoutError()
        chunk = await asyncio.wait_for(reader.read(65536), timeout=remain)
        if not chunk:
            break
        data += chunk
        if min_bytes <= 1:
            break
    return data

# --------- Stats ---------

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

    def note_error(self, s: str, connect: bool = False) -> None:
        if self.first_error is None:
            self.first_error = s
        if connect:
            self.connect_errors += 1
        else:
            self.op_errors += 1

def summarize(stats: Stats, started_ns: int) -> str:
    dur_s = (now_ns() - started_ns) / 1e9
    ops = stats.ops
    rps = ops / dur_s if dur_s > 0 else 0.0
    bps = (stats.bytes_sent + stats.bytes_recv) / dur_s if dur_s > 0 else 0.0

    lat = stats.lat_ms
    p50 = percentile(lat, 0.50) if lat else float("nan")
    p95 = percentile(lat, 0.95) if lat else float("nan")
    p99 = percentile(lat, 0.99) if lat else float("nan")
    lat_avg = statistics.mean(lat) if lat else float("nan")

    return (
        f"duration={dur_s:.2f}s  "
        f"connects={stats.connects}  connect_err={stats.connect_errors}  "
        f"ops={ops}  op_err={stats.op_errors}  "
        f"rps={rps:.1f}  throughput={fmt_num(bps)}/s  "
        f"lat_ms(avg/p50/p95/p99)={lat_avg:.3f}/{p50:.3f}/{p95:.3f}/{p99:.3f}"
    )

# --------- Workers ---------

async def churn_worker(
    wid: int,
    host: str,
    port: int,
    duration_s: float,
    msg_size: int,
    verify: str,
    timeout: float,
    stats: Stats,
    stop_event: asyncio.Event,
) -> None:
    # Churn: connect -> send N=1 -> recv -> close -> repeat
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
            writer.write(payload)
            await asyncio.wait_for(writer.drain(), timeout=timeout)
            stats.bytes_sent += len(payload)

            # Lue jotain takaisin (serveristä riippuen)
            resp = await read_some(reader, min_bytes=1, timeout=timeout)
            stats.bytes_recv += len(resp)

            dt_ms = (now_ns() - t0) / 1e6
            stats.lat_ms.append(dt_ms)
            stats.ops += 1

            if verify == "echo":
                # echo: odota että resp sisältää payloadin kokonaan; jos serveri lähettää muuta, tämä failaa
                if resp != payload:
                    raise AssertionError(f"verify=echo failed: resp_len={len(resp)}")
            elif verify == "prefix":
                # prefix: odota että resp alkaa payloadin alusta (tai että payload alkaa respistä)
                # Tämä on löysä, mutta sopii ACK-tyyppisiin toteutuksiin jos laitat payloadin alkuun tunnisteen.
                if not (resp.startswith(payload[: min(8, len(payload))]) or payload.startswith(resp[: min(8, len(resp))])):
                    raise AssertionError(f"verify=prefix failed: resp_len={len(resp)}")

        except Exception as e:
            stats.note_error(f"[worker {wid}] op failed: {e!r}", connect=False)
        finally:
            try:
                writer.close()
                await writer.wait_closed()
            except Exception:
                pass


async def steady_worker(
    wid: int,
    host: str,
    port: int,
    duration_s: float,
    msg_size: int,
    messages_per_conn: int,
    verify: str,
    timeout: float,
    stats: Stats,
    stop_event: asyncio.Event,
) -> None:
    # Steady: one connection per worker, send/recv in a loop
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
                writer.write(payload)
                await asyncio.wait_for(writer.drain(), timeout=timeout)
                stats.bytes_sent += len(payload)

                resp = await read_some(reader, min_bytes=1, timeout=timeout)
                stats.bytes_recv += len(resp)

                dt_ms = (now_ns() - t0) / 1e6
                stats.lat_ms.append(dt_ms)
                stats.ops += 1

                if verify == "echo" and resp != payload:
                    raise AssertionError(f"verify=echo failed: resp_len={len(resp)}")
                if verify == "prefix":
                    if not (resp.startswith(payload[: min(8, len(payload))]) or payload.startswith(resp[: min(8, len(resp))])):
                        raise AssertionError(f"verify=prefix failed: resp_len={len(resp)}")

    except Exception as e:
        stats.note_error(f"[worker {wid}] op failed: {e!r}", connect=False)
    finally:
        try:
            writer.close()
            await writer.wait_closed()
        except Exception:
            pass


async def reporter(stats: Stats, started_ns: int, every_s: float, stop_event: asyncio.Event) -> None:
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

# --------- Main ---------

async def run(args) -> int:
    stats = Stats()
    started_ns = now_ns()
    stop_event = asyncio.Event()

    def on_sigint():
        stop_event.set()

    loop = asyncio.get_running_loop()
    for sig in (signal.SIGINT, signal.SIGTERM):
        try:
            loop.add_signal_handler(sig, on_sigint)
        except NotImplementedError:
            pass

    tasks = []
    if args.report_every > 0:
        tasks.append(asyncio.create_task(reporter(stats, started_ns, args.report_every, stop_event)))

    # Launch workers
    for i in range(args.clients):
        if args.mode == "churn":
            tasks.append(asyncio.create_task(
                churn_worker(i, args.host, args.port, args.duration, args.msg_size, args.verify, args.timeout, stats, stop_event)
            ))
        else:
            tasks.append(asyncio.create_task(
                steady_worker(i, args.host, args.port, args.duration, args.msg_size, args.messages_per_conn, args.verify, args.timeout, stats, stop_event)
            ))

    try:
        await asyncio.wait_for(asyncio.gather(*tasks), timeout=args.duration + 5.0)
    except asyncio.TimeoutError:
        # Normal if workers run until duration
        stop_event.set()
    finally:
        stop_event.set()

    print("\n=== SUMMARY ===")
    print(summarize(stats, started_ns))
    if stats.first_error:
        print(f"first_error={stats.first_error}")
        return 2
    return 0


def parse_args() -> argparse.Namespace:
    p = argparse.ArgumentParser(description="STCP stress test client (asyncio).")
    p.add_argument("--host", default="127.0.0.1")
    p.add_argument("--port", type=int, required=True)
    p.add_argument("--mode", choices=["churn", "steady"], default="churn",
                   help="churn=connect/send/recv/close loop; steady=keep connections open")
    p.add_argument("--clients", type=int, default=50)
    p.add_argument("--duration", type=float, default=10.0)
    p.add_argument("--msg-size", type=int, default=64)
    p.add_argument("--messages-per-conn", type=int, default=10, help="steady mode only")
    p.add_argument("--verify", choices=["none", "prefix", "echo"], default="none")
    p.add_argument("--timeout", type=float, default=2.0)
    p.add_argument("--report-every", type=float, default=1.0)
    return p.parse_args()


if __name__ == "__main__":
    args = parse_args()
    try:
        rc = asyncio.run(run(args))
    except KeyboardInterrupt:
        rc = 130
    sys.exit(rc)

