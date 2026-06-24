import asyncio
import argparse
import asyncio
import socket
from Crypto.Cipher import AES
from Crypto.Util.Padding import pad, unpad
import hashlib
import base64
import os

import stcpClient
import stcpServer
import utils
import time

messages = 0
def messageHandler(messageIn: str):
    global messages
    messages += 1
    msg = f"{messages}: {messageIn}"
    return msg

class TestClient:
    def __init__(self, host: str, port: int, message_handler):
        self.client = stcpClient.StcpClient(host, port, message_handler)

    def start(self):
        print(f"Starting client. {self.client}..");
        self.client.connect();
        print(f"Started.");
        msgnro = 0
        while True:
            msg = f"{msgnro} viesti .. tämä on salattu?"
            msgnro += 1
            self.client.send_message(msg);
            incoming = self.client.recv_message()
            utils.log_and_get_string_from_bytes("Incoming plain", incoming)
            time.sleep(5)
 


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Test STCP client")
    parser.add_argument("--host", type=str, default="localhost", help="Server hostname")
    parser.add_argument("--port", type=int, default=8888, help="Server port")
    
    args = parser.parse_args()
    
    test_client = TestClient(args.host, args.port, messageHandler)
    test_client.start()
    print(f"hmm.. serve quit ?");
  
