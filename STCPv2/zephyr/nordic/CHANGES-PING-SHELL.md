# STCP shell IPv4 ping

Added:

- `stcp ping <ip|name>`
- optional packet count: `stcp ping <host> 10`
- optional timeout: `stcp ping <host> 10 5000`
- IPv4 DNS resolution
- ICMP Echo Request/Reply using Zephyr native networking
- packet loss and RTT min/avg/max summary

No W5500 driver, polling, SPI, benchmark or LTE transport code was modified.
