# tcp_server.py
import socket

HOST = "0.0.0.0"
PORT = 7777

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen(1)
    print(f"Listening on {PORT}...")

    conn, addr = s.accept()
    print("Client connected from:", addr)

    with conn:
        while True:
            data = conn.recv(1024)
            if not data:
                print("Client disconnected")
                break

            print("RX:", data)
            conn.sendall(b"pong\n")
