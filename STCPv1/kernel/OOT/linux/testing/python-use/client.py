#!/usr/bin/env python3
import argparse
import os
import signal
import socket
import sys
import time


def log(msg: str) -> None:
    ts = time.strftime("%H:%M:%S")
    print(f"[CLIENT {ts}] {msg}", flush=True)


def make_client_socket(proto: int, timeout: float) -> socket.socket:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, proto)
    s.settimeout(timeout)
    return s


def run_app_only(host: str, port: int, proto: int, timeout: float, payload: bytes, expect_echo: bool) -> int:
    s = make_client_socket(proto, timeout)
    try:
        log(f"connect {host}:{port} proto={proto}")
        s.connect((host, port))
        log(f"send {len(payload)} bytes: {payload!r}")
        s.sendall(payload)
        data = s.recv(4096)
        log(f"recv {len(data)} bytes: {data!r}")

        if expect_echo:
            expected = b"OK:" + payload
            if data != expected:
                log(f"ERROR: unexpected reply, expected {expected!r}")
                return 2
        else:
            if not data.startswith(b"OK"):
                log("ERROR: reply doesn't start with OK")
                return 2

        return 0

    except socket.timeout:
        log("ERROR: timeout")
        return 3
    except ConnectionRefusedError:
        log("ERROR: connection refused")
        return 4
    except ConnectionResetError:
        log("ERROR: connection reset")
        return 5
    except OSError as e:
        log(f"ERROR: socket error: {e!r}")
        return 6
    finally:
        try:
            s.close()
        except Exception:
            pass


def main() -> int:
    ap = argparse.ArgumentParser(description="STCP client test (app-only).")
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=2225)
    ap.add_argument("--proto", type=int, default=253)
    ap.add_argument("--timeout", type=float, default=30.0)
    ap.add_argument("--mode", choices=["app"], default="app")
    ap.add_argument("--payload", default="hello-from-client", help="Payload string to send.")
    ap.add_argument("--loops", type=int, default=1, help="How many times to connect/send/recv.")
    ap.add_argument("--sleep", type=float, default=0.0, help="Sleep between loops.")
    ap.add_argument("--expect-echo", action="store_true",
                    help="Expect server reply to be exactly OK:<payload>.")
    ap.add_argument("--kill-server-after", type=float, default=None,
                    help="After N seconds, SIGKILL all python3 processes (kill test).")
    args = ap.parse_args()

    payload = args.payload.encode("utf-8", errors="replace")

    if args.kill_server_after is not None:
        # Brutal kill test for teardown
        def killer():
            time.sleep(args.kill_server_after)
            log(f"SIGKILL python3 (kill-server-after={args.kill_server_after})")
            os.system("sudo pkill -9 -f python3")

        import threading
        threading.Thread(target=killer, daemon=True).start()

    rc = 0
    for i in range(args.loops):
        log(f"loop {i+1}/{args.loops}")
        rc = run_app_only(args.host, args.port, args.proto, args.timeout, payload, args.expect_echo)
        if rc != 0:
            log(f"FAILED on loop {i+1} rc={rc}")
            return rc
        if args.sleep > 0:
            time.sleep(args.sleep)

    log("OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
