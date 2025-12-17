#!/usr/bin/env python3
import secrets
import string
import socket
import sys
import time
from cryptography.hazmat.primitives.asymmetric import x25519
from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.kdf.hkdf import HKDF
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.ciphers.aead import AESGCM

SERVER_HOST = "127.0.0.1"
SERVER_PORT = 2525
STCP_PROTO = 253
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

HS1 = b"STCP_HS1|"
HS2 = b"STCP_HS2|"

AESHDR = b"STCP_AES|"

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

def aes_encrypt(key: bytes, plaintext: bytes) -> bytes:
    aes = AESGCM(key)
    nonce = secrets.token_bytes(12)  # AES-GCM standard nonce
    ct = aes.encrypt(nonce, plaintext, None)  # AAD=None
    return nonce + ct

def aes_decrypt(key: bytes, packet: bytes) -> bytes:
    if len(packet) < 12 + 16:
        raise ValueError("packet too short")
    nonce = packet[:12]
    ct = packet[12:]
    aes = AESGCM(key)
    return aes.decrypt(nonce, ct, None)


def random_string(length=32):
    alphabet = string.ascii_letters + string.digits
    return ''.join(secrets.choice(alphabet) for _ in range(length))

if len(sys.argv) > 1:
    SERVER_PORT = int(sys.argv[1])
    
if len(sys.argv) > 2:
    STCP_PROTO = int(sys.argv[2])

def main():
    print(f"[CLIENT] Luodaan STCP-soketti proto={STCP_PROTO}")
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, STCP_PROTO)
    peer_pub = ""
    shared_key = ""

    try:
        print(f"[CLIENT] Yhdistetään {SERVER_HOST}:{SERVER_PORT} ...")
        s.connect((SERVER_HOST, SERVER_PORT))
        print("[CLIENT] Yhdistetty!")

        # Testaa että non-blocking handshake toimii: pieni sleep
        time.sleep(10)

        # Lähetä testidataa (jos Rust-puoli ei vielä vaadi mitään erityistä formaattia)
        msg = HS1 + MY_PUBKEY
        print(f"[CLIENT] Lähetetään {len(msg)} tavua: {msg!r}")
        sent = s.send(msg)
        print(f"[CLIENT] send() palautti {sent}")

        server_pubkey = ""
        # Yritä lukea jotain takaisin (tämä voi blokata jos serveri ei vastaa)
        try:
            s.settimeout(30.0)
            print(f"[CLIENT] recv() timeout 30 sec")
            data = s.recv(128)
            print(f"[CLIENT] recv() -> {len(data)} tavua: {data!r}")

            if data.startswith(HS2):
                dlen = len(HS2)
                server_pubkey = data[dlen:]
                print(f"[CLIENT] RECV public key from server: -> {len(server_pubkey)} tavua: {server_pubkey.hex()}")
            else:
                raise OSError(f"Handshake failed: expected {HS2!r}, got {data!r}")

            peer_pub = x25519.X25519PublicKey.from_public_bytes(server_pubkey)
            shared_key = MY_PRIKEY.exchange(peer_pub)
            print(f"[CLIENT] HS Shared {shared_key.hex()}")            
            print(f"[CLIENT] Handshake ok.")            

        except socket.timeout:
            print("[CLIENT] recv timeout – ei vastausta (ok, jos serveri ei vielä lähetä mitään)")

        theAesKey = derive_aes_key(shared_key)

        msg = b"TERVEISIA PYTHONI ASIAKKAALTA"
        aesmsg = AESHDR + aes_encrypt(theAesKey, msg)
        print(f"[CLIENT] Lähetetään {len(aesmsg)} tavua: {aesmsg.hex()}")
        sent = s.send(aesmsg)
        print(f"[CLIENT] send() palautti {sent}")

        s.settimeout(30.0)
        print(f"[CLIENT] recv() timeout 30 sec")

        data = s.recv(4096)
        print(f"[CLIENT] recv() -> {len(data)} tavua: {data!r}")

    except OSError as e:
        print(f"[CLIENT] Virhe socket-operaatiossa: {e}", file=sys.stderr)
    finally:
        print("[CLIENT] Suljetaan soketti.")
        s.close()
        print("[CLIENT] Done.")

if __name__ == "__main__":
    main()
