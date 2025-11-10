#!/usr/bin/env python3
import socket
import sys
import time

HOST = "127.0.0.1"
PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 5678  # vaihda tähän testin käyttämä portti

def main():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # sallii nopean restartin
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen(16)
    print(f"[stcp-test-server] listening on {HOST}:{PORT}")

    try:
        while True:
            conn, addr = s.accept()
            print(f"[stcp-test-server] accepted from {addr}")
            # ei tehdä mitään, pidetään vähän aikaa auki
            try:
                conn.settimeout(5.0)
                _ = conn.recv(4096)  # jos testiohjelma lähettää jotain
            except socket.timeout:
                pass
            except ConnectionResetError:
                pass
            finally:
                conn.close()
                print(f"[stcp-test-server] closed connection from {addr}")
    except KeyboardInterrupt:
        print("\n[stcp-test-server] shutting down")
    finally:
        s.close()

if __name__ == "__main__":
    main()

