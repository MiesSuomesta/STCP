# stcp_hub.py
import socket, threading

clients = []

def handle(c):
    while True:
        data = c.recv(4096)
        if not data:
            break
        for o in clients:
            if o is not c:
                o.sendall(data)
    clients.remove(c)
    c.close()

s = socket.socket()
s.bind(("0.0.0.0", 6789))
s.listen()

print("STCP HUB listening on :6789")

while True:
    c, a = s.accept()
    print("client:", a)
    clients.append(c)
    threading.Thread(target=handle, args=(c,), daemon=True).start()

