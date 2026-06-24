#!/usr/bin/env python3
import argparse
import os
import socket
import sys
import time
from typing import Tuple


def log(msg: str) -> None:
    ts = time.strftime("%H:%M:%S")
    print(f"[SERVER {ts}] {msg}", flush=True)


def make_server_socket(host: str, port: int, proto: int) -> socket.socket:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, proto)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    # Optional: allow quick restarts even with TIME_WAIT (mostly for TCP; harmless here)
    try:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
    except OSError:
        pass
    s.bind((host, port))
    s.listen(16)
    return s


def handle_conn_app_only(conn: socket.socket, addr: Tuple[str, int], timeout: float, echo: bool) -> None:
    conn.settimeout(timeout)
    log(f"accepted {addr}, waiting for application data...")
    data = conn.recv(4096)
    log(f"recv {len(data)} bytes: {data!r}")
    if echo:
        conn.sendall(b"OK:" + data)
        log("sent OK:... echo")
    else:
        conn.sendall(b"OK")
        log("sent OK")


def main() -> int:
    ap = argparse.ArgumentParser(description="STCP server test (app-only or legacy handshake).")
    ap.add_argument("--host", default="0.0.0.0")
    ap.add_argument("--port", type=int, default=2225)
    ap.add_argument("--proto", type=int, default=253)
    ap.add_argument("--timeout", type=float, default=30.0)
    ap.add_argument("--mode", choices=["app"], default="app",
                    help="app = application-only (no userspace handshake).")
    ap.add_argument("--echo", action="store_true", help="Echo back received payload with OK: prefix.")
    ap.add_argument("--once", action="store_true", help="Handle exactly one connection and exit.")
    args = ap.parse_args()

    log(f"starting on {args.host}:{args.port} proto={args.proto} mode={args.mode}")
    srv = make_server_socket(args.host, args.port, args.proto)

    try:
        while True:
            log("listening...")
            conn, addr = srv.accept()
            try:
                if args.mode == "app":
                    handle_conn_app_only(conn, addr, args.timeout, args.echo)
                else:
                    raise RuntimeError("Unsupported mode")
            except socket.timeout:
                log("ERROR: conn timed out during recv/send")
            except ConnectionResetError:
                log("ERROR: connection reset by peer")
            except OSError as e:
                log(f"ERROR: socket error: {e!r}")
            except Exception as e:
                log(f"ERROR: unexpected: {e!r}")
            finally:
                try:
                    conn.close()
                except Exception:
                    pass

            if args.once:
                break

    finally:
        srv.close()
        log("server exit")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
