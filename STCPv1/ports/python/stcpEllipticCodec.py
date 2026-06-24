import socket
import struct
import base64
from cryptography.hazmat.primitives.asymmetric import ec
from cryptography.hazmat.primitives import serialization, hashes
from cryptography.hazmat.primitives.kdf.hkdf import HKDF
from cryptography.hazmat.primitives.asymmetric.utils import encode_dss_signature, decode_dss_signature

import stcpCommon
import stcpServer
import stcpClient
import utils

STCP_AES_KEY_SIZE_IN_BYTES = 32
STCP_AES_IV_SIZE_IN_BYTES = 16

class StcpEllipticCodec:
    def __init__(self):
        self.private_key = ec.generate_private_key(ec.SECP256R1())
        self.public_key = self.private_key.public_key()
        self.theSharedKey = ""
        self.theAESkeyDerived = ""

    def public_key_to_bytes(self) -> bytes:
        return self.public_key.public_bytes(
            encoding=serialization.Encoding.X962,
            format=serialization.PublicFormat.UncompressedPoint
        )

    def private_key_to_bytes(self) -> bytes:
        return self.private_key.private_bytes(
            encoding=serialization.Encoding.Raw,
            format=serialization.PrivateFormat.Raw,
            encryption_algorithm=serialization.NoEncryption()
        )

    def bytes_to_public_key(self, public_key_bytes: bytes) -> ec.EllipticCurvePublicKey:
        try:
            # Varmista, että avain on uncompressed-muodossa
            if public_key_bytes[0] != 0x04:
                raise ValueError("Unsupported elliptic curve point type")
            return ec.EllipticCurvePublicKey.from_encoded_point(ec.SECP256R1(), public_key_bytes)
        except ValueError as e:
            print(f"Virhe avaimen deserialisoimisessa: {e}")
            return None

    def derive_shared_key_based_aes_key(self, shared_secret: bytes) -> bytes:
        hkdf = HKDF(
            algorithm=hashes.SHA256(),
            length=STCP_AES_KEY_SIZE_IN_BYTES,
            salt=None,
            info=b'stcp aes key',
        )
        return hkdf.derive(shared_secret)
    
    def sign_message(self, message: str) -> bytes:
        signature = self.private_key.sign(
            message.encode(),
            ec.ECDSA(hashes.SHA256())
        )
        return signature
    
    def verify_signature(self, message: str, signature: bytes, public_key: ec.EllipticCurvePublicKey) -> bool:
        try:
            public_key.verify(signature, message.encode(), ec.ECDSA(hashes.SHA256()))
            return True
        except:
            return False
        
    def compute_shared_secret(self, peerPublicKey) -> bytes:
        return self.private_key.exchange(ec.ECDH(), peerPublicKey)
    
    def compute_shared_secret_as_bytes(self, peerPublicKey) -> bytes:
        return self.compute_shared_secret(peerPublicKey)


