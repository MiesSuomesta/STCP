STCPv2 Zephyr RXSTAT overlay - 2026-08-26

Purpose:
  Diagnose whether Linux->Zephyr handshake bytes ever reach the native Zephyr socket.

Adds per-socket counters:
  rxstat_calls
  rxstat_eagain
  rxstat_bytes
  rxstat_last_errno

Logging:
  RXSTAT summary every 500 ms while recv() is returning no data.
  Immediate RXSTAT + RXDIAG DATA/CORE logs whenever data arrives.
  Summary on EOF, fatal error, stop and thread exit.

Expected examples:
  RXSTAT reason=periodic fd=0 calls=423 eagain=423 bytes=0 last_errno=11 ...
  RXSTAT reason=data fd=0 calls=17 eagain=15 bytes=144 last_errno=0 ...

This overlay preserves the RX-ready startup barrier from the previous RXDIAG overlay.
No Rust core or wire protocol changes.
