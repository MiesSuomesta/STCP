# STCP transport echo server

Ports:

- TCP: 19000
- TLS 1.2+: 19001
- Linux AF_STCP / protocol 253: 19002

Generate a development certificate and start all listeners:

```bash
./generate-cert.sh
sudo python3 echo_server.py
```

Start only the first plain-TCP test:

```bash
python3 echo_server.py --no-tls --no-stcp
```

The STCP listener requires the STCP Linux kernel module to be loaded. Binding to
port 19002 may require root depending on the module implementation.

Open the selected inbound ports in the firewall/NAT. For the first nRF9151 test,
point `CONFIG_ECHO_SERVER_HOST` and `CONFIG_ECHO_SERVER_PORT` in
`stcp-mqtt/src/echo_benchmark.c` to a publicly reachable server and TCP port
19000.
