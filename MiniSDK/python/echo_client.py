#!/usr/bin/env python3
"""Linux STCP-TCP echo client. Standard library only."""
import ctypes
import socket
import struct
import sys

libc = ctypes.CDLL(None, use_errno=True)
libc.connect.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_uint]
libc.connect.restype = ctypes.c_int


def exchange(client, payload):
    for offset in range(0, len(payload), 4096):
        part = payload[offset:offset + 4096]
        client.sendall(part)
        reply = bytearray()
        while len(reply) < len(part):
            data = client.recv(len(part) - len(reply))
            if not data:
                raise OSError('connection closed before complete echo')
            reply.extend(data)
        if reply != part:
            raise ValueError('echo mismatch')


def main():
    if not 2 <= len(sys.argv) <= 4:
        raise ValueError('usage: echo_client.py IPv4 [port] [message]')
    host = socket.inet_pton(socket.AF_INET, sys.argv[1])
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 7000
    if not 1 <= port <= 65535:
        raise ValueError('port must be 1..65535')
    payload = (sys.argv[3] if len(sys.argv) > 3 else 'Hello STCP!').encode('utf-8')
    if not payload:
        raise ValueError('message must not be empty')
    with socket.socket(45, socket.SOCK_STREAM, 253) as client:
        address = ctypes.create_string_buffer(
            struct.pack('=H', socket.AF_INET) + struct.pack('!H', port) + host + bytes(8))
        if libc.connect(client.fileno(), address, 16) < 0:
            raise OSError(ctypes.get_errno(), 'connect failed')
        exchange(client, payload)
    print(f'PASS: {len(payload)} bytes echoed unchanged')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError) as exc:
        sys.exit(str(exc))
