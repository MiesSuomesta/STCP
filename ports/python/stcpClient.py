import asyncio
import socket
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad
import hashlib
import base64

import stcpCommon
import stcpEllipticCodec
import utils
import tcpClient

class StcpClient:
    def __init__(self, host: str, port: int, the_message_handler):
        self.host = host
        self.port = port
        self.unsecure_message_handler = the_message_handler
        self.tcpClient = None;
        self.hasPublicKey = False
        self.theCommon = stcpCommon.StcpCommon()
        self.theEC = stcpEllipticCodec.StcpEllipticCodec()
        self.theSharedSecret = None
        self.handshaker = self.do_handshake_client
        self.tcpClient = tcpClient.TcpClient(self.host, self.port, self.secure_message_handler, self.handshaker)

        self.peerPublicKey = ""
        self.theSharedSecret = ""
        self.theAESkey = ""
        self.theDerivedAESkey = ""
        
    def debug_keys(self, msg):
        print(f"DEBUG Keys at {msg}: peerPublicKey // {self.peerPublicKey} //")
        print(f"DEBUG Keys at {msg}: theSharedSecret // {self.theSharedSecret} //")
        print(f"DEBUG Keys at {msg}: theAESkey // {self.theAESkey} //")
        print(f"DEBUG Keys at {msg}: theDerivedAESkey // {self.theDerivedAESkey} //")


    def do_handshake_client(self, theSock: socket):
        while True:
            print(f"[Initial] Sending Public key....")
            out = utils.log_and_get_bytes_from_string("Public Key of client", \
                    self.theEC.public_key_to_bytes())
            print(f"[Initial] Sending out: // {len(out)} // {out}")
            theSock.sendall(out)
            incomingRawData = theSock.recv(10000)
            self.peerPublicKey, self.theSharedSecret = \
                    self.theCommon.checkForPublicKey(incomingRawData, self.theEC)
            print(f"[Initial] Got return from PublicKey check: {self.peerPublicKey}, {self.theSharedSecret}")
            if self.peerPublicKey != False:
                print(f"Got Public Key of peer {self.peerPublicKey}");
                self.set_aes_key(self.theSharedSecret)
                self.debug_keys("at client handshake")
                return self.peerPublicKey, self.theSharedSecret

    def set_aes_key(self, theKey):
        key = self.theEC.derive_shared_key_based_aes_key(theKey)
        print(f"theDerivedAESkey set: {theKey} => {self.theDerivedAESkey}")
        self.theDerivedAESkey = key

    def secure_message_handler(self, incomingRawData, theSock: socket):
        print(f"[Client/AES] ..at aes traffic: // {len(incomingRawData)} // {incomingRawData}")

        # Aes traffic
        utils.log_and_get_string_from_bytes("incomiing AES traffic",incomingRawData)

        decrypted = self.theCommon.the_secure_message_transfer_incoming( \
            msg_incoming_crypted=incomingRawData, the_aes_preshared_key=self.theDerivedAESkey)
        
        utils.log_and_get_string_from_bytes("Decrypted AES traffic", decrypted)
        decryptedOut = self.unsecure_message_handler(decrypted)
        utils.log_and_get_string_from_bytes("Plain AES traffic out", decryptedOut)

        crypted = self.theCommon.the_secure_message_transfer_outgoing( \
            decryptedOut, the_aes_preshared_key=self.theDerivedAESkey)
        utils.log_and_get_string_from_bytes("Crypted AES traffic out", crypted)

        return crypted
    
        # TCP connection: AES
    def connect(self):
        print(f"Connecting.....")
        self.tcpClient.connect()
        print(f"Connected.")

    def send_message(self, message: str):
        crypted = self.theCommon.the_secure_message_transfer_outgoing(message, the_aes_preshared_key=self.theDerivedAESkey)
        self.tcpClient.send_message(crypted)

    def recv_message(self):
        crypted = self.tcpClient.recv_message()
        plain = self.theCommon.the_secure_message_transfer_incoming(crypted, the_aes_preshared_key=self.theDerivedAESkey)
        return plain
    
    def close(self):
        self.tcpClient.stop();
        print("Connection closed")

# Esimerkki viestin käsittelystä
def handle_message(message: str):
    print("Received:", message)

# Testataan asiakas
if __name__ == "__main__":
    tcp_client = StcpClient("127.0.0.1", 8888, handle_message)
    tcp_client.connect()
