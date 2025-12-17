#!/usr/bin/env python3
import socket
import struct
import threading
import sys

IPPROTO_STCP = 253
HOST, PORT = "0.0.0.0", int(sys.argv[1])

MAX_FRAME = 32 * 1024 * 1024

def recv_exact(conn: socket.socket, n: int) -> bytes:
    buf = b""
    while len(buf) < n:
        chunk = conn.recv(n - len(buf))
        if not chunk:
            raise EOFError("peer closed")
        buf += chunk
    return buf

def handle_client(conn: socket.socket, addr):
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
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, IPPROTO_STCP)
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen(1024)
    print(f"STCP framed THREADED server listening on {HOST}:{PORT} proto={IPPROTO_STCP}")

    while True:
        conn, addr = s.accept()
        t = threading.Thread(target=handle_client, args=(conn, addr), daemon=True)
        t.start()

if __name__ == "__main__":
    main()

