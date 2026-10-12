STCPv7 native UDP RX instrumentation (diagnostics only)
Source: stcp-project-11.10.2026-215858-STCP.zip
Path: zephyr/nordic/application/src/echo_benchmark.c

Apply from STCP repository root:
  unzip -o stcpv7-udp-rx-observability.zip

Adds per-direction BENCH_UDP_RX_DIAG counters and first sequence gap timing.
No change to UDP handling, socket cleanup, transport order, ports, GAP, buffers,
or protocol behavior. Only benchmark application-layer counters.

LIMITATION: These counters only see datagrams delivered to zsock_recv().
They do not measure W5500 hardware drops, Zephyr driver drops, or net_pkt
allocation failures. For those, separate driver/Zephyr-stack instrumentation
against the exact deployed Zephyr tree is required.

Expected grep:
  grep 'BENCH_UDP_RX_DIAG\|BENCH_UDP_BAD_FRAME\|BENCH_NET_DIAG' minicom.log
