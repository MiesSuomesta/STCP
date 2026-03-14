#!/usr/bin/env python3

import socket
import struct
import argparse
import os
import time

STCP_MAGIC = 0x53544350  # "STCP"

def build_frame(version, msg_type, payload):
    header = struct.pack(
        ">IIII",
        version,
        STCP_MAGIC,
        msg_type,
        len(payload)
    )
    return header + payload


def send_frame(host, port, frame):

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))

    print(f"Connected to {host}:{port}")

    # ------------------------
    # 1️⃣ Dummy STCP handshake
    # ------------------------

    fake_pubkey = os.urandom(64)

    print("Sending dummy STCP public key (64 bytes)")
    s.sendall(fake_pubkey)

    # pieni tauko auttaa debugissa
    time.sleep(0.1)

    # ------------------------
    # 2️⃣ STCP frame
    # ------------------------

    print(f"Sending STCP frame ({len(frame)} bytes)")
    s.sendall(frame)

    # pidetään socket hetki auki
    time.sleep(0.5)

    print("Frame sent")

    s.close()


def main():

    parser = argparse.ArgumentParser(description="Send STCP mock frame")

    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=7777)
    parser.add_argument("--type", type=int, default=1)
    parser.add_argument("--version", type=int, default=1)
    parser.add_argument("--payload", default="hello")

    args = parser.parse_args()

    payload = args.payload.encode()

    frame = build_frame(
        args.version,
        args.type,
        payload
    )

    send_frame(args.host, args.port, frame)


if __name__ == "__main__":
    main()
