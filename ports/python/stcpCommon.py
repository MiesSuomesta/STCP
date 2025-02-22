from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad
import os
import base64
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend
import socket

import stcpAesCodec
import stcpEllipticCodec

class StcpCommon:

    def checkForPublicKey(self, paramDataIn: str, theEC: stcpEllipticCodec.StcpEllipticCodec) -> str:

        if paramDataIn is not None:
            msgLen = len(paramDataIn)
            print(f"Got Public key? {msgLen} .. {paramDataIn}")
            if msgLen == 65:
                print(f"Content is proper length ...")
                theByte = paramDataIn[0]
                hasPubkey = theByte == 4
                print(f"Got Public key? {msgLen} && byte: {theByte} => {hasPubkey}")
                peerPublicKey = theEC.bytes_to_public_key(paramDataIn)
                if peerPublicKey is not None:
                    theCommonSecret = theEC.compute_shared_secret(peerPublicKey)
                    theAESKey = theEC.derive_shared_key_based_aes_key(theCommonSecret)
                    print(f"Handshake return: {peerPublicKey}, {theCommonSecret}")
                    return peerPublicKey, theCommonSecret
                else:
                    print(f"Handshake failed: {peerPublicKey}, {theCommonSecret}")
                    return False, False

            else:
                print(f"Not public key.. lenght mismatch: {msgLen}")
                return False, False
        else:
            return False, False

    def the_secure_message_transfer_incoming(self, msg_incoming_crypted: bytes, the_aes_preshared_key: str) -> bytes:

        if msg_incoming_crypted is None:
            print("Error: crypted message was none!");
            return None

        if the_aes_preshared_key is None:
            print("Error: the_aes_preshared_key was none!");
            return None

        AESCodec = stcpAesCodec.stcpAesCodec()
        print(f"the_aes_key: {len(the_aes_preshared_key)} // {the_aes_preshared_key}")

        the_aes_key = AESCodec.getPaddedKey(the_aes_preshared_key)
        print(f"the_aes_key: {len(the_aes_key)} // {the_aes_key}")
        the_incoming_iv = msg_incoming_crypted[:stcpAesCodec.STCP_AES_IV_SIZE_IN_BYTES]
        print(f"the_incoming_iv: {len(the_incoming_iv)} // {the_incoming_iv}")
        the_encrypted_message = msg_incoming_crypted[stcpAesCodec.STCP_AES_IV_SIZE_IN_BYTES:]
        print(f"the_encrypted_message: {len(the_encrypted_message)} // {the_encrypted_message}")
        decrypted_message = AESCodec.decrypt(the_encrypted_message, the_aes_key, the_incoming_iv)
        print(f"decrypted_message: // {decrypted_message} //")
        return decrypted_message
    
    def the_secure_message_transfer_outgoing(self, msg_outgoing_plain: bytes, the_aes_preshared_key: str) -> bytes:

        if msg_outgoing_plain is None:
            return None

        AESCodec = stcpAesCodec.stcpAesCodec()
        the_aes_key = AESCodec.getPaddedKey(the_aes_preshared_key)
        the_outgoing_iv = os.urandom(stcpAesCodec.STCP_AES_IV_SIZE_IN_BYTES)
        
        crypted_message = AESCodec.encrypt(msg_outgoing_plain, the_aes_key, the_outgoing_iv)
        return the_outgoing_iv + crypted_message
    
    def encrypt(self, plaintext, key, iv) -> bytes:

        if plaintext is None:
            return None

        key = key.encode("utf-8")
        iv = iv.encode("utf-8")
        print(f"encrypt in {len(key)} // {len(iv)} /// {key} // {iv}");
        cipher = Cipher(algorithms.AES(key), modes.CBC(iv), backend=default_backend())
        encryptor = cipher.encryptor()
        padded_plaintext = plaintext + b" " * (16 - len(plaintext) % 16)
        return encryptor.update(padded_plaintext) + encryptor.finalize()
    
    def decrypt(self, ciphertext, key, iv) -> bytes:

        if ciphertext is None:
            return None

        key = key.encode("utf-8")
        iv = iv.encode("utf-8")
        print(f"decrypt in {len(key)} // {len(iv)} /// {key} // {iv}");
        cipher = Cipher(algorithms.AES(key), modes.CBC(iv), backend=default_backend())
        decryptor = cipher.decryptor()
        return decryptor.update(ciphertext) + decryptor.finalize()

if __name__ == "__main__":
    print("STCP Common module is ready.")
