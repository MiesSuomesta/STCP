# STCP/UDP shared TX fairness v12

This version uses the rolled-back `kernel(2).zip` tree as its base.

## Change

Accepted UDP children share one kernel UDP socket. Previously every child called
`kernel_sendmsg()` concurrently, so up to 16 independent reliability windows
could burst into the same root socket at once.

The root carrier now owns an asynchronous TX scheduler:

- ACK/control frames use a high-priority FIFO.
- DATA frames remain FIFO within each connection ID.
- After 8 DATA frames from one session, the scheduler selects the oldest queued
  DATA frame from another session.
- Only the root TX worker calls `kernel_sendmsg()` in production mode.
- Queue size and queue drops are bounded and reported in final carrier stats.
- Fault-injection test mode still uses the direct-send path.

## Test

```bash
HOST=192.168.1.199 PORT=19002 CLIENTS_LIST="16 8 4 2 1" \
  bash benchmark/test-stcp-udp-tx-fairness-v12.sh
```

Check final stats for `tx_queue_drops=0`.
