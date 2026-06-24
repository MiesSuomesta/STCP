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

def messageHandler(messageIn: str):
    print(f"Got message: $messageIn")
    return f"OK Got it: {messageIn}"


class TestServer:
    def __init__(self, host: str, port: int, message_handler):
        self.server = stcpServer.StcpServer(host, port, message_handler)

    def start(self):
        print(f"Starting server...");
        self.server.start()
        print(f"Started.");

def main():
    parser = argparse.ArgumentParser(description="Test STCP Server")
    parser.add_argument("--host", type=str, default="localhost", help="Server hostname")
    parser.add_argument("--port", type=int, default=8888, help="Server port")
    
    args = parser.parse_args()
    
    test_server = TestServer(args.host, args.port, messageHandler)
    test_server.start()
    print(f"hmm.. serve quit ?");
  
