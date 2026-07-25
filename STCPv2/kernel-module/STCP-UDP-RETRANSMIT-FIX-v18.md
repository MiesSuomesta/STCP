# STCP UDP retransmit recovery v18

Base: user supplied kernel(3).zip.

## Root cause addressed

The observed ~32 second tail matches the previous retry ladder:
250/1000 baseline variants aside, the original configuration used roughly
1s + 2s + 4s + repeated 5s retries before marking the socket failed.
Additionally, the C delayed-work callback permanently stopped whenever a
single Rust tick returned a negative transient error.

## Changes

- retransmit delayed work uses `system_highpri_wq` on x86 and Raspberry Pi
- transient negative `stcp_rust_tick()` results no longer stop the worker
- UDP initial RTO: 1000 ms -> 250 ms
- minimum RTO: 100 ms -> 50 ms
- maximum RTO: 5000 ms -> 1000 ms
- retransmit burst: 4 -> 8 oldest expired frames
- retry ceiling: 8 -> 32
- reaching retry ceiling no longer closes the SOCK_STREAM session; recovery
  remains active at capped RTO and userspace timeout retains final authority
- event 334 reports a frame that reached the retry ceiling

## Test

Run the existing clean baseline test first:

```bash
HOST=192.168.1.199 PORT=19002 CLIENTS_LIST="1 2 4 8 16" \
  bash benchmark/test-stcp-udp-direct-tx-burst-v13.sh
```

Then run five 16-client repetitions with the orchestrator and compare:
- receive timeout count
- p95/p99 RTT
- event 308 cadence
- event 334 occurrence
- final UDP carrier tx/rx balance
