# STCP/UDP RX session fairness v10

The shared UDP listener previously used one global FIFO for data frames. Under
16 concurrent 1 MiB transfers, one connection could keep the dispatcher busy
long enough for other sessions to reach a full 128-frame send window.

v10 keeps the existing high-priority control queue, but applies a per-session
budget to the data queue:

- process at most 8 consecutive data frames for one connection ID;
- then prefer the oldest queued frame from another connection;
- preserve FIFO ordering within every individual connection;
- fall back to the queue head when no other session is waiting.

No wire-format or Rust protocol changes are introduced by this fix.
