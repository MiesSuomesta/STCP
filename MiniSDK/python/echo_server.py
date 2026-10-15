#!/usr/bin/env python3
"""Linux STCP-TCP echo server; no third-party dependencies."""
import ctypes
import errno
import socket
import struct
import os
import sys

libc = ctypes.CDLL(None, use_errno=True)
libc.bind.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_uint]
libc.bind.restype = ctypes.c_int
libc.accept.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_void_p]
libc.accept.restype = ctypes.c_int


def accept_client(server):
    # CPython accept() cannot size an address for the custom AF_STCP family.
    # No peer address is needed. Use libc and explicit AF_INET metadata for
    # the Python wrapper; the descriptor itself remains an STCP socket.
    while True:
        fd = libc.accept(server.fileno(), None, None)
        if fd >= 0:
            try:
                return socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0, fileno=fd)
            except BaseException:
                os.close(fd)
                raise
        code = ctypes.get_errno()
        if code in (errno.EINTR, errno.EPIPE, errno.ECONNRESET, errno.ECONNABORTED,
                    errno.ENOTCONN, errno.ETIMEDOUT, errno.ECONNREFUSED):
            continue
        raise OSError(code, os.strerror(code))


def echo(client):
    while True:
        data = client.recv(65536)
        if not data:
            return
        client.sendall(data)


def main():
    if len(sys.argv) > 2:
        raise ValueError('usage: echo_server.py [port]')
    port = int(sys.argv[1]) if len(sys.argv) == 2 else 7000
    if not 1 <= port <= 65535:
        raise ValueError('port must be 1..65535')
    with socket.socket(45, socket.SOCK_STREAM, 253) as server:
        # Python cannot encode an IPv4 tuple for a custom socket family.
        address = ctypes.create_string_buffer(
            struct.pack('=H', socket.AF_INET) + struct.pack('!H', port)
            + bytes(12))
        if libc.bind(server.fileno(), address, 16) < 0:
            code = ctypes.get_errno()
            raise OSError(code, errno.errorcode.get(code, 'bind failed'))
        server.listen(16)
        print(f'STCP-TCP echo listening on 0.0.0.0:{port}', flush=True)
        while True:
            client = accept_client(server)
            with client:
                try:
                    echo(client)
                except OSError as exc:
                    print(f'client: {exc}', file=sys.stderr)


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
    except (OSError, ValueError) as exc:
        sys.exit(str(exc))
