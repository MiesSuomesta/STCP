#!/usr/bin/env python3
from __future__ import annotations

import argparse
import ctypes
import os
import socket
import ssl
import struct
import threading

AF_STCP = 45
STCP_PROTO = 253
HEADER = struct.Struct("!I")


class SockAddrIn(ctypes.Structure):
    _fields_ = [
        ("sin_family", ctypes.c_ushort),
        ("sin_port", ctypes.c_ushort),
        ("sin_addr", ctypes.c_uint32),
        ("sin_zero", ctypes.c_ubyte * 8),
    ]


libc = ctypes.CDLL(None, use_errno=True)
libc.bind.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_uint]
libc.bind.restype = ctypes.c_int
libc.accept.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_void_p]
libc.accept.restype = ctypes.c_int


def native_error(operation: str) -> None:
    err = ctypes.get_errno()
    raise OSError(err, f"{operation}: {os.strerror(err)}")


def stcp_address(host: str, port: int) -> SockAddrIn:
    packed = socket.inet_aton(host)
    return SockAddrIn(
        sin_family=socket.AF_INET,
        sin_port=socket.htons(port),
        sin_addr=ctypes.c_uint32.from_buffer_copy(packed).value,
        sin_zero=(ctypes.c_ubyte * 8)(*([0] * 8)),
    )


def bind_listener(listener: socket.socket, mode: str, host: str, port: int) -> None:
    if mode != "stcp":
        listener.bind((host, port))
        return
    address = stcp_address(host, port)
    if libc.bind(listener.fileno(), ctypes.byref(address), ctypes.sizeof(address)) < 0:
        native_error("STCP bind")


def accept_connection(listener: socket.socket, mode: str) -> socket.socket:
    if mode != "stcp":
        conn, _ = listener.accept()
        return conn
    fd = libc.accept(listener.fileno(), None, None)
    if fd < 0:
        native_error("STCP accept")
    return socket.socket(AF_STCP, socket.SOCK_STREAM, STCP_PROTO, fileno=fd)


def recv_exact(conn: socket.socket, size: int) -> bytes:
    chunks: list[bytes] = []
    remaining = size
    while remaining:
        chunk = conn.recv(remaining)
        if not chunk:
            raise ConnectionError("peer closed")
        chunks.append(chunk)
        remaining -= len(chunk)
    return b"".join(chunks)


def worker(conn: socket.socket) -> None:
    try:
        while True:
            raw_length = recv_exact(conn, HEADER.size)
            (length,) = HEADER.unpack(raw_length)
            if length > 64 * 1024 * 1024:
                raise ValueError(f"payload too large: {length}")
            payload = recv_exact(conn, length)
            conn.sendall(raw_length + payload)
    except (ConnectionError, OSError, ValueError):
        pass
    finally:
        try:
            conn.close()
        except OSError:
            pass


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode", choices=("tcp", "tls", "stcp"), required=True)
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, required=True)
    parser.add_argument("--cert")
    parser.add_argument("--key")
    args = parser.parse_args()

    if args.mode == "stcp":
        listener = socket.socket(AF_STCP, socket.SOCK_STREAM, STCP_PROTO)
    else:
        listener = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        listener.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    bind_listener(listener, args.mode, args.host, args.port)
    listener.listen(256)

    context: ssl.SSLContext | None = None
    if args.mode == "tls":
        if not args.cert or not args.key:
            parser.error("TLS requires --cert and --key")
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.minimum_version = ssl.TLSVersion.TLSv1_3
        context.load_cert_chain(args.cert, args.key)

    print(f"benchmark server: mode={args.mode} listen={args.host}:{args.port}", flush=True)

    while True:
        conn = accept_connection(listener, args.mode)
        if context is not None:
            try:
                conn = context.wrap_socket(conn, server_side=True)
            except ssl.SSLError:
                conn.close()
                continue
        threading.Thread(target=worker, args=(conn,), daemon=True).start()


if __name__ == "__main__":
    raise SystemExit(main())
