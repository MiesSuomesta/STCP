STCPv2 Zephyr persistent lifecycle summary diagnostic overlay — 2026-08-26

Purpose
-------
Robot may not preserve the serial output of a PASSed test when the next test
fails. This overlay keeps the previous AF_STCP socket teardown result in static
debug state and prints it at the beginning of the next socket create.

New markers
-----------
  LIFESUM PREV ...
  LIFESUM CREATE BEGIN ...
  LIFECYCLE CREATE DONE create_seq=...

stage_mask
----------
  0x01 close entered
  0x02 RX stop returned
  0x04 Rust context release stage completed
  0x08 carrier free returned
  0x10 socket free entered
  0x20 socket free returned / close complete

A completely finished previous close prints stage_mask=0x3f and complete=1.
The summary also preserves rx_running/rx_stop before close and immediately
after stcp_v2_rx_stop(), plus close duration.

Scope
-----
Only module-v2/src/stcp_v2_socket.c is changed. No wire protocol, shared Rust
core, ByteQueue, or carrier behavior is changed.
