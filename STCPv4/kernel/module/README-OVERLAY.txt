STCPv4 TCP benchmark bracketing overlay v2
===========================================

Changes:
  - TOTAL/A/B/C/D timing in rust/src/session.rs
  - start/stop aggregate only; no printk on hot path
  - NO benchmark dump from stcp_rust_exit()
  - /sys/module/stcp/parameters/benchmark_dump trigger

Dump while module remains loaded:

  echo 1 | sudo tee /sys/module/stcp/parameters/benchmark_dump >/dev/null
  sudo dmesg | grep STCP-BENCH

The parameter automatically returns to 0 after a dump, so it can be
triggered repeatedly.
