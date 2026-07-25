# STCP/UDP shared listener RX dispatch fix

The UDP root/listener receiver no longer executes Rust frame processing directly
inside the socket-drain loop.

## Changes

- The socket receiver copies UDP datagrams into a bounded in-kernel queue and
  immediately returns to `kernel_recvmsg()`.
- A dedicated dispatcher kthread invokes `stcp_rust_carrier_receive_from()`.
- Control packets (ACK, handshake and other non-data frames) use a priority FIFO.
- Data packets retain FIFO order in a separate queue.
- Queue overflow and final queue-drop counters are logged.
- TCP carrier behaviour is unchanged.

The queue is bounded at 32768 datagrams. This prevents unbounded allocation while
providing enough burst absorption for concurrent one-megabyte benchmark sessions.

## Regression test

```bash
HOST=192.168.1.199 PORT=19002 \
CLIENTS_LIST="1 2 4 8 16" \
bash benchmark/test-stcp-udp-shared-rx.sh
```
