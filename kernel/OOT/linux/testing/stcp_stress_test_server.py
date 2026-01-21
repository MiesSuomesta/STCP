#!/usr/bin/env python3
import socket
import struct
import threading
import sys

HOST, PORT = "0.0.0.0", int(sys.argv[1])
USE_PROTOCOL_NRO = 253

if (len(sys.argv) > 1):
    USE_PROTOCOL_NRO = int(sys.argv[2])

MAX_FRAME = 32 * 1024 * 1024

KMSG = "/dev/kmsg"

def klog(msg: str, level: int = 6) -> None:
    # level: 0..7 (0=emerg, 3=err, 6=info, 7=debug)
    line = f"<{level}>[py] {msg}\n"
    with open(KMSG, "w", buffering=1) as f:
        f.write(line)

def recv_exact(conn: socket.socket, n: int) -> bytes:
    buf = b""
    while len(buf) < n:
        chunk = conn.recv(n - len(buf))
        if not chunk:
            raise EOFError("peer closed")
        buf += chunk
    return buf

def handle_client(conn: socket.socket, addr):
    klog(f"got connect {conn} && {addr}...");
    try:
        # Optional: per-connection timeout to avoid stuck clients
        conn.settimeout(60.0)
        while True:
            hdr = recv_exact(conn, 4)
            (ln,) = struct.unpack("!I", hdr)
            if ln > MAX_FRAME:
                raise ValueError(f"frame too big: {ln}")
            data = recv_exact(conn, ln)
            conn.sendall(struct.pack("!I", ln) + data)
    except (EOFError, ConnectionError, OSError, ValueError, socket.timeout):
        pass
    finally:
        try:
            conn.close()
        except Exception:
            pass

def main():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, USE_PROTOCOL_NRO)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen(1024)
    klog(f"STCP framed THREADED server listening on {HOST}:{PORT} proto={USE_PROTOCOL_NRO}")

    while True:
        conn, addr = s.accept()
        klog(f"Connection from {addr}...");
        t = threading.Thread(target=handle_client, args=(conn, addr), daemon=True)
        t.start()

if __name__ == "__main__":
    klog("Starting STCP server...");
    main()

