STCPv2 Zephyr benchmark checkpoint overlay
===========================================

Extract in the STCPv2 repository root:

  cd ~/STCP/STCPv2
  unzip -o stcp-zephyr-benchmark-checkpoint-overlay.zip

Changed:
  zephyr/nordic/application/src/echo_benchmark.c

Low-volume upload checkpoints:

  U01  connect enter
  U02  connect return
  U03  request send enter
  U04  request send return
  U05  upload stream enter
  U06  upload stream return
  U07  final reply receive enter
  U08  final reply receive return

Request helper:
  R01  send_all enter
  R02  send_all return

Reply helper:
  P01  recv_all enter
  P02  recv_all return
  P03  malformed reply
  P04  valid reply/status

Useful grep:
  grep 'BENCHCHK' <zephyr-console-log>

No per-chunk logging, sleeps, protocol changes or memory changes are added.
