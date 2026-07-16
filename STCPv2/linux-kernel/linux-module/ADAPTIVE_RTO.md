# Adaptive RTT/RTO reliability

This version replaces the fixed retransmission timeout with a per-connection
RTT estimator and a per-frame exponential backoff.

## Estimator

The implementation uses integer arithmetic:

- first sample:
  - `SRTT = RTT`
  - `RTTVAR = RTT / 2`
- subsequent samples:
  - `RTTVAR = (3 * RTTVAR + |SRTT - RTT|) / 4`
  - `SRTT = (7 * SRTT + RTT) / 8`
- `RTO = SRTT + 4 * RTTVAR`

RTO is clamped to 100–3000 ms. The retransmit worker currently runs every
100 ms, so wire measurements have 100 ms granularity.

## Karn's algorithm

Frames that have been retransmitted do not contribute RTT samples because the
ACK cannot be associated unambiguously with the original transmission.

## Backoff and failure

Each pending frame stores its own timeout. A timeout doubles that frame's RTO,
up to 3000 ms. A connection enters the error state after eight retries.

## Statistics

The Rust context tracks:

- sent frames
- acknowledged frames
- retransmissions
- duplicates
- reordered frames
- timeout failures
- RTT sample count
- SRTT, RTTVAR and current RTO

The C release path prints these values to the kernel log before releasing the
Rust context.

## Fault injection

Module parameters:

- `drop_first_data=1`
- `drop_percent=10`
- `drop_percent=30`
- `delay_first_data_ms=250`
- `duplicate_first_data=1`
- `reorder_first_pair=1`

`drop_percent` uses a deterministic sequence, making failures reproducible.

## Test

```bash
make LLVM=1 V=1 test-reliability
```

The target runs baseline, first-packet drop, 10% loss, 30% loss, 250 ms delay,
duplicate and reorder cases.
