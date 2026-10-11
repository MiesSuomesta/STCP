STCPv7 Zephyr bench diagnostics patched against src-11102026_195519.zip.
Replace echo_benchmark.c and add stcp_bench_diag.h alongside it.
Other original files are included unchanged. No CMake change required.
Logs BENCH_NET_DIAG after each upload/download/full run (also on failures).
Counts raw zsock_send/zsock_recv calls, including BEN2 control and metrics traffic.
recv_calls are socket calls, rx_packets are successful recv calls, NOT wire packets.
rx_drops=0 is NOT evidence of zero dropped packets; driver counter is not wired.
Time uses k_cycle_get_32 and k_cyc_to_us_floor32: single-wrap assumption;
very long blocking syscalls may overflow cycle counter and underreport duration.
The calls are nonblocking in the benchmark's data path.
No Zephyr build was run in this environment.
