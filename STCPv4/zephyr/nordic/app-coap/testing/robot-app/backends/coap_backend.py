#!/usr/bin/env python3
import socket
import sys

HOST = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
PORT = int(sys.argv[2]) if len(sys.argv) > 2 else 5683
PAYLOAD = b"STCP_COAP_OK"

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind((HOST, PORT))
print(f"COAP_TEST_BACKEND_READY {HOST}:{PORT}", flush=True)

while True:
    data, peer = sock.recvfrom(2048)
    if len(data) < 4:
        print(f"COAP_TEST_DROP short={len(data)}", flush=True)
        continue

    version = data[0] >> 6
    msg_type = (data[0] >> 4) & 0x03
    tkl = data[0] & 0x0f
    code = data[1]
    mid_hi, mid_lo = data[2], data[3]
    if version != 1 or tkl > 8 or len(data) < 4 + tkl:
        print(f"COAP_TEST_DROP malformed len={len(data)}", flush=True)
        continue

    token = data[4:4+tkl]
    print(
        f"COAP_TEST_REQUEST bytes={len(data)} type={msg_type} code={code} "
        f"mid={(mid_hi << 8) | mid_lo} token={token.hex()}",
        flush=True,
    )

    # ACK + 2.05 Content, preserving message ID and token.
    first = (1 << 6) | (2 << 4) | tkl
    response = bytes([first, 69, mid_hi, mid_lo]) + token + b"\xff" + PAYLOAD
    sock.sendto(response, peer)
    print(
        f"COAP_TEST_RESPONSE bytes={len(response)} mid={(mid_hi << 8) | mid_lo} "
        f"payload={PAYLOAD.decode()}",
        flush=True,
    )
