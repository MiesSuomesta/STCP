
#
# https://github.com/MiesSuomesta/STCP/ port to Python
#
import socket
import time
import tcpServer

import stcpCommon
import stcpEllipticCodec
import utils

class StcpServer:

    def __init__(self, host='localhost', port=8888, message_handler=None):
        self.host = host
        self.port = port
        self.clients = [ message_handler ]
        self.unsecure_message_handler = message_handler
        self.theSharedSecret = None
        self.theAESkey = None
        self.theCommon = stcpCommon.StcpCommon()
        self.theEC = stcpEllipticCodec.StcpEllipticCodec()
        self.handshaker = self.do_handshake_server
        self.server = tcpServer.TcpServer(host, port, self.handle_connection, self.handshaker)

        self.peerPublicKey = ""
        self.theSharedSecret = ""
        self.theDerivedAESkey = ""

    def set_aes_key(self, theSharedKey):
        key = self.theEC.derive_shared_key_based_aes_key(theSharedKey)
        self.theDerivedAESkey = key
        print(f"theDerivedAESkey set: {theSharedKey} => {self.theDerivedAESkey}")

    def debug_keys(self, msg):
        print(f"DEBUG Keys at {msg}: peerPublicKey // {self.peerPublicKey} //")
        print(f"DEBUG Keys at {msg}: theSharedSecret // {self.theSharedSecret} //")
        print(f"DEBUG Keys at {msg}: theDerivedAESkey // {self.theDerivedAESkey} //")

    def do_handshake_server(self, theSock: socket):
        while True:
            print(f"[Initial] Sending Public key....")
            out = utils.log_and_get_bytes_from_string("Public Key of server", \
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
                self.debug_keys("at server handshake")
                return self.peerPublicKey, self.theSharedSecret

    def start(self):
        print(f"Server starting at {self.host}:{self.port}")
        self.server.start()
        print(f"Server ok at {self.host}:{self.port}")

    def handle_connection(self, client_sock, addr):
        print(f"Waiting for AES traffic...")
        hasAEStraffic = True
        while hasAEStraffic:
            # Allow very bick tcp packets
            print("Waiting for AES packet");
            incomingRawData = client_sock.recv(10000)
            msgLen = len(incomingRawData)
            utils.log_and_get_string_from_bytes("AES in", incomingRawData)
            # Aes traffic
            ret = self.secure_AES_message_handler(incomingRawData)
            if ret != None:
                client_sock.sendall(ret)

    def secure_AES_message_handler(self, incomingRaw: str):
        if incomingRaw is None:
            return None

        # TCP connection: AES
        utils.log_and_get_string_from_bytes("Encrypted AES traffic in", incomingRaw)
        self.debug_keys("at message handling")
        decrypted = self.theCommon.the_secure_message_transfer_incoming(msg_incoming_crypted=incomingRaw, the_aes_preshared_key=self.theDerivedAESkey)
        utils.log_and_get_string_from_bytes("Decrypted AES traffic in", decrypted)
        
        decryptedOut = self.unsecure_message_handler(decrypted)

        utils.log_and_get_string_from_bytes("Decrypted AES traffic out", decryptedOut)
        crypted = self.theCommon.the_secure_message_transfer_outgoing(decryptedOut, the_aes_preshared_key=self.theDerivedAESkey)

        utils.log_and_get_string_from_bytes("Encrypted AES traffic out", crypted)
        return crypted

if __name__ == "__main__":
    server = StcpServer()
    server.start_server()
