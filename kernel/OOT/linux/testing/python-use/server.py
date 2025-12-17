#!/usr/bin/env python3
import secrets
import string
import socket
import sys
import time
from cryptography.hazmat.primitives.asymmetric import x25519
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.kdf.hkdf import HKDF
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
from cryptography.hazmat.primitives import hashes

HOST = "0.0.0.0"
PORT = 2525
PROTO_STCP = 253
handshake_done = False

HS1 = b"STCP_HS1|"
HS2 = b"STCP_HS2|"

# Private key
MY_PRIKEY = x25519.X25519PrivateKey.generate()

# Public key bytes (32 B)
MY_PUBKEY = MY_PRIKEY.public_key().public_bytes(
    encoding=serialization.Encoding.Raw,
    format=serialization.PublicFormat.Raw
)

HS1OUT = HS1 + MY_PUBKEY

def derive_aes_key(shared: bytes) -> bytes:
    # 32 bytes = AES-256 key
    hkdf = HKDF(
        algorithm=hashes.SHA256(),
        length=32,
        salt=None,                 # ok testipenkissä; myöhemmin mieluummin salt/nonce
        info=b"STCP-AESGCM-v1",    # tärkeä: domain separation
    )
    return hkdf.derive(shared)


if len(sys.argv) > 1:
    PORT = int(sys.argv[1])

if len(sys.argv) > 2:
    PROTO_STCP = int(sys.argv[2])


def hexdump(data: bytes) -> str:
    return " ".join(f"{b:02x}" for b in data)

def main():
    print(f"[STCP-SERVER] Starting on {HOST}:{PORT} proto={PROTO_STCP}")

    # AF_INET, SOCK_STREAM, proto=253 (STCP)
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM, PROTO_STCP)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    try:
        srv.bind((HOST, PORT))
    except OSError as e:
        print(f"[STCP-SERVER] bind() failed: {e}")
        sys.exit(1)

    srv.listen(5)
    print("[STCP-SERVER] Listening... (Ctrl+C katkaisee)")

    try:
        while True:
            try:
                print("\n[STCP-SERVER] Accepthing connections")
                conn, addr = srv.accept()
            except KeyboardInterrupt:
                print("\n[STCP-SERVER] KeyboardInterrupt -> exit")
                break

            print(f"[STCP-SERVER] New connection from {addr}")


            with conn:
                handshake_done = False
                client_pubkey = ""

                while True:
                    try:
                        data = conn.recv(4096)
                    except OSError as e:
                        print(f"[STCP-SERVER] recv() error: {e}")
                        break

                    if not data:
                        print("[STCP-SERVER] Client closed connection")
                        break

                    print(f"[STCP-SERVER] Received {len(data)} bytes")
                    print(f"  HEX : {hexdump(data)}")
                    print(f"  TEXT: {data.decode('utf-8', errors='replace')!r}")

                    # --- Phase 1 handshake (plain) ---
                    if not handshake_done:
                        if data.startswith(HS1):
                            print("[STCP-SERVER] HS: starts with HS1 -> sending HS2")
                            dlen = len(HS1)
                            client_pubkey = data[dlen:]
                            print(f"[STCP-SERVER] HS: Client pub key: {client_pubkey.hex()}")
                            HS2OUT = HS2 + MY_PUBKEY;
                            try:
                                conn.sendall(HS2OUT)
                            except OSError as e:
                                print(f"[STCP-SERVER] sendall(HS2) error: {e}")
                                break
                            handshake_done = True
                            continue
                        else:
                            print(f"[STCP-SERVER] HS: expected {HS1!r}, got {data!r} -> closing")
                            break

                    # --- Normal echo path ---

                    peer_pub = x25519.X25519PublicKey.from_public_bytes(client_pubkey)
                    shared_key = MY_PRIKEY.exchange(peer_pub)
                    print(f"[STCP-SERVER] HS Shared {shared_key.hex()}")            

                    reply = b"STCP-SERVER-ACK: " + data
                    try:
                        conn.sendall(reply)
                    except OSError as e:
                        print(f"[STCP-SERVER] sendall() error: {e}")
                        break

                    print(f"[STCP-SERVER] Sent {len(reply)} bytes back")

    finally:
        srv.close()
        print("[STCP-SERVER] Socket closed")

if __name__ == "__main__":
    main()

