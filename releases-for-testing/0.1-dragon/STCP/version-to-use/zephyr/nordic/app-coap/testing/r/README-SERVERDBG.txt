STCP BEN2 debug server overlay - 2026-08-26

Extract this ZIP in zephyr/nordic/application/testing/robot-v2 so that:
  server/stcp_v2_bench_server.c
  server/Makefile
replace the existing files.

Build:
  make -C server clean all

The BEN2 wire/application protocol is unchanged.
Added stderr diagnostics include:
  SERVERDBG MAIN/BIND/LISTEN
  SERVERDBG ACCEPT ENTER/RETURN
  SERVERDBG CONN HANDLE ENTER/RETURN
  SERVERDBG BENCH upload/download/full progress
  SERVERDBG IO read/write results
  SERVERDBG CONN CLOSE ENTER/RETURN
  SERVERDBG CONN DONE next_accept=1

This is intended to prove whether the server fully closes connection #1,
returns to accept(), and accepts connection #2 during sequential Zephyr tests.
