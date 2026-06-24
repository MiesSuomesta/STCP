#!/usr/bin/env python3
import argparse
import socket
import struct
import sys
import time

from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.kdf.hkdf import HKDF
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.ciphers.aead import AESGCM


# ==============================
# STCP WIRE FORMAT (17 bytes)
# u32 version
# u64 tag
# u8  type
# u32 payload_len
# ==============================
BYPASS_AES = True

HEADER_LEN = 17
STCP_VERSION = 1
STCP_ECDH_PUB_LEN = 64


def log(msg):
    ts = time.strftime("%H:%M:%S")
    print(f"[SERVER {ts}] {msg}", flush=True)


def recv_exact(conn, n):
    buf = b""
    while len(buf) < n:
        chunk = conn.recv(n - len(buf))
        if not chunk:
            raise ConnectionError("peer closed")
        buf += chunk
    return buf


def parse_header(buf):
    if len(buf) != HEADER_LEN:
        raise ValueError("invalid header size")

    version = int.from_bytes(buf[0:4], "big")
    tag = int.from_bytes(buf[4:12], "big")
    msg_type = buf[12]
    payload_len = int.from_bytes(buf[13:17], "big")

    return version, tag, msg_type, payload_len


def send_frame(conn, msg_type, payload, tag=0):
    header = (
        STCP_VERSION.to_bytes(4, "big") +
        tag.to_bytes(8, "big") +
        bytes([msg_type]) +
        len(payload).to_bytes(4, "big")
    )
    conn.sendall(header + payload)
    log(f"sent frame type={msg_type} len={len(payload)}")


def handle_connection(conn, addr, echo):
    log(f"accepted {addr}")

    log(f"Receiving raw pub key from client {STCP_ECDH_PUB_LEN} bytes")
    # === 1. Receive client raw pubkey (64 bytes) ===
    client_pub_raw = recv_exact(conn, STCP_ECDH_PUB_LEN)

    # === 2. Generate server ECDH key ===
    server_private = ec.generate_private_key(ec.SECP256R1())
    server_public = server_private.public_key()

    server_pub_bytes = server_public.public_bytes(
        encoding=serialization.Encoding.X962,
        format=serialization.PublicFormat.UncompressedPoint
    )

    pubklen = len(server_pub_bytes) - 1

    log(f"Sending raw pub key to client {pubklen} bytes")
    # Send raw 64 bytes (without 0x04 prefix)
    conn.sendall(server_pub_bytes[1:])

    # === 3. Compute shared secret ===
    log(f"Computing shared key 1....")
    client_pub = ec.EllipticCurvePublicKey.from_encoded_point(
        ec.SECP256R1(),
        b"\x04" + client_pub_raw
    )

    shared_secret = server_private.exchange(ec.ECDH(), client_pub)
    log(f"Computing shared key 2....")

    hkdf = HKDF(
        algorithm=hashes.SHA256(),
        length=32,
        salt=None,
        info=b"stcp",
    )
    aes_key = hkdf.derive(shared_secret)
    aesgcm = AESGCM(aes_key)

    log(f"AES key len   {len(aes_key)}")
    log(f"AES key bytes {aes_key.hex()}")

    log("SERVER: Handshake done ===================================================")
    log("SERVER: Handshake done ===================================================")
    log("SERVER: Handshake done ===================================================")
    log("SERVER: Handshake done ===================================================")

    # === 4. AES frame loop ===
    while True:
        log(f"Waiting header....")
        header = recv_exact(conn, HEADER_LEN)
        print("HEADER:", header.hex())

        version, tag, msg_type, payload_len = parse_header(header)
        print("PARSED:", version, tag, msg_type, payload_len)

        encrypted_payload = recv_exact(conn, payload_len)
        print("ENCRYPTED:", encrypted_payload.hex())

        iv = encrypted_payload[:12]
        ciphertext = encrypted_payload[12:]

        plaintext = aesgcm.decrypt(iv, ciphertext, None)
        print("DECRYPTED:", plaintext)

        log(f"Sending response...")
        if echo:
            send_frame(conn, msg_type, b"OK:" + plaintext, tag)
        else:
            send_frame(conn, msg_type, b"OK", tag)
        log(f"Message handled.")


def main():
    parser = argparse.ArgumentParser(description="STCP Test Server")
    parser.add_argument("--host", default="0.0.0.0")
    parser.add_argument("--port", type=int, default=7777)
    parser.add_argument("--proto", type=int, default=6,
                        help="IP protocol number (6 = TCP, 253 = custom)")
    parser.add_argument("--loop", action="store_true",
                        help="Keep accepting new connections")
    parser.add_argument("--echo", action="store_true",
                        help="Echo decrypted payload")
    args = parser.parse_args()

    log(f"Starting server on {args.host}:{args.port} proto={args.proto}")

    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM, args.proto)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind((args.host, args.port))
    srv.listen(16)

    try:
        while True:
            log("listening...")
            conn, addr = srv.accept()
            try:
                handle_connection(conn, addr, args.echo)
            except Exception as e:
                log(f"connection error: {e}")
            finally:
                conn.close()

            if not args.loop:
                break

    finally:
        srv.close()
        log("server exit")


if __name__ == "__main__":
    main()
