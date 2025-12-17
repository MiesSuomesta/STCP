#!/usr/bin/env python3
import socket
import sys

IPPROTO_STCP = 253   # varmista että tämä täsmää kernelissä

HOST = "0.0.0.0"
PORT = 5566

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM, IPPROTO_STCP)
s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
s.bind((HOST, PORT))
s.listen(128)

print(f"STCP server listening on {HOST}:{PORT} (proto {IPPROTO_STCP})")

while True:
    conn, addr = s.accept()
    print("Accepted:", addr)
    try:
        while True:
            data = conn.recv(4096)
            if not data:
                break
            # Echo / ACK – kernel salaa automaattisesti
            conn.sendall(b"STCP-SERVER-ACK:" + data)
    except Exception as e:
        print("conn error:", e)
    finally:
        conn.close()

