#!/usr/bin/env python3
import socket
import sys

HOST = sys.argv[1] if len(sys.argv) > 1 else "127.0.0.1"
PORT = int(sys.argv[2]) if len(sys.argv) > 2 else 1883


def recv_exact(conn, n):
    out = bytearray()
    while len(out) < n:
        chunk = conn.recv(n - len(out))
        if not chunk:
            raise EOFError
        out.extend(chunk)
    return bytes(out)


def recv_packet(conn):
    first = recv_exact(conn, 1)[0]
    multiplier = 1
    remaining = 0
    while True:
        b = recv_exact(conn, 1)[0]
        remaining += (b & 0x7f) * multiplier
        if not (b & 0x80):
            break
        multiplier *= 128
        if multiplier > 128 ** 3:
            raise ValueError("bad MQTT remaining length")
    return first, recv_exact(conn, remaining)


def parse_publish(first, body):
    if len(body) < 2:
        return "", b""
    tlen = (body[0] << 8) | body[1]
    if len(body) < 2 + tlen:
        return "", b""
    topic = body[2:2+tlen].decode("utf-8", errors="replace")
    pos = 2 + tlen
    qos = (first >> 1) & 0x03
    if qos:
        pos += 2
    return topic, body[pos:]

srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
srv.bind((HOST, PORT))
srv.listen(8)
print(f"MQTT_TEST_BROKER_READY {HOST}:{PORT}", flush=True)

while True:
    conn, peer = srv.accept()
    print(f"MQTT_TEST_ACCEPT peer={peer[0]}:{peer[1]}", flush=True)
    with conn:
        try:
            while True:
                first, body = recv_packet(conn)
                ptype = first >> 4
                if ptype == 1:  # CONNECT
                    print(f"MQTT_TEST_CONNECT bytes={len(body)}", flush=True)
                    conn.sendall(b"\x20\x02\x00\x00")  # CONNACK accepted
                    print("MQTT_TEST_CONNACK_OK", flush=True)
                elif ptype == 3:  # PUBLISH
                    topic, payload = parse_publish(first, body)
                    print(
                        f"MQTT_TEST_PUBLISH topic={topic} bytes={len(payload)} "
                        f"payload={payload.decode('utf-8', errors='replace')}",
                        flush=True,
                    )
                elif ptype == 12:  # PINGREQ
                    conn.sendall(b"\xd0\x00")
                    print("MQTT_TEST_PINGRESP", flush=True)
                elif ptype == 14:  # DISCONNECT
                    print("MQTT_TEST_DISCONNECT", flush=True)
                    break
                else:
                    print(f"MQTT_TEST_PACKET type={ptype} bytes={len(body)}", flush=True)
        except EOFError:
            print("MQTT_TEST_EOF", flush=True)
        except Exception as exc:
            print(f"MQTT_TEST_ERROR {exc}", flush=True)
