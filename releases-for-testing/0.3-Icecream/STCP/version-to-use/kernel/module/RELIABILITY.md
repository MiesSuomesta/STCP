# STCPv2 UDP reliability layer

This version completes the existing reliability implementation:

- per-direction 64-bit sequence numbers;
- cumulative ACKs;
- eight-frame send window;
- retained encrypted wire frames for retransmission;
- 300 ms fixed retransmission timeout;
- five retries before entering Error state;
- duplicate suppression;
- bounded out-of-order buffering;
- client and accepted-child retransmit workers;
- deterministic drop, duplicate and reorder injection.

Run:

```bash
make LLVM=1 V=1 test-reliability
```

## UDP datagram size fix

The encrypted STCP frame payload is capped at 60 KiB. A 64 KiB plaintext
payload exceeded UDP's 65507-byte datagram limit after the STCP header, nonce,
and authentication tag were added, causing `send()` to fail with `EMSGSIZE`.
