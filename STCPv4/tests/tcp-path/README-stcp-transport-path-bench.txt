STCP transport path benchmark

Run:
  ./stcp_transport_path_bench --help

Supported:
  tcp, stcp-tcp, tls, udp, stcp-udp

Build:
  gcc -O2 -Wall -Wextra stcp_transport_path_bench.c -o stcp_transport_path_bench -lssl -lcrypto
