Corrected STCP transport path benchmark

Supported transports:
  tcp
  tls
  udp
  stcp-tcp
  stcp-udp

Important STCP mapping:
  stcp-tcp: AF_STCP + SOCK_STREAM + protocol 253
  stcp-udp: AF_STCP + SOCK_STREAM + protocol 254

Build:
  gcc -O2 -Wall -Wextra stcp_tcp_path_bench.c -o stcp_tcp_path_bench -lssl -lcrypto

Help:
  ./stcp_tcp_path_bench --help

STCP-UDP example:
  ./stcp_tcp_path_bench server stcp-udp 0.0.0.0 19956
  ./stcp_tcp_path_bench client stcp-udp SERVER_IP 19956 128044 256 3
