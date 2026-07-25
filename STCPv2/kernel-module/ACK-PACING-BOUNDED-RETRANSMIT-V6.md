# STCP/UDP ACK pacing and bounded retransmit v6

This version balances the two failure modes seen in the previous builds:

- v4 retransmitted 16 frames per timer pass and produced a retransmit storm.
- v5 ACKed every frame and retransmitted only one frame per pass, producing an ACK storm and recovery that was too slow under 8-16 clients.

## Changes

- Normal cumulative ACK every 8 contiguous UDP data frames.
- `DataChunkEnd` is ACKed immediately.
- Duplicate and out-of-order frames force an immediate cumulative ACK.
- Retransmit timer sends at most the 4 oldest expired pending frames per pass.
- Existing shared RX dispatcher, cumulative ACK recovery and send-window partial writes remain enabled.

## Test

```bash
HOST=192.168.1.199 \
PORT=19002 \
CLIENTS_LIST="16 8 4 2 1" \
bash benchmark/test-stcp-udp-balanced-recovery-v6.sh
```

Expected debug behaviour:

- normal ACK progress: `event=309`
- bounded retransmit: `event=308 arg0<=4`
- no sustained per-frame ACK callback storm
- no sustained `event=301 ... arg0=128 arg1=0` without subsequent `event=309`
