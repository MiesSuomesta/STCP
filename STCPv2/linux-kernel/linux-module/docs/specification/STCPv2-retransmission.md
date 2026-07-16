# STCPv2 send window and retransmission

## Window

Each endpoint keeps at most eight unacknowledged DATA frames. `send()` returns
`-EAGAIN` if the new message would exceed the current window.

## Pending frames

Each transmitted DATA frame is retained until an ACK acknowledges its sequence
number. ACK `N` cumulatively frees pending frames with sequence `<= N`.

## Retransmission timer

Every socket owns a Linux `delayed_work` item. It calls the Rust session tick
every 100 ms. A pending frame is retransmitted after three ticks (about 300 ms).

A frame may be retransmitted five times. Exceeding the retry limit moves the
session to the error state.

## Duplicate handling

Retransmitted DATA frames may arrive after the original frame was accepted.
The receiver must ACK an already accepted sequence again without delivering its
payload twice.

## Test mode

Loading the module with:

```text
drop_first_data=1
```

drops the first DATA frame exactly once. The normal basic test therefore
exercises the retransmission path.

## Deterministic fault-injection modes

The module supports three UDP-only test parameters:

- `drop_first_data=1`: silently drops the first DATA frame;
- `duplicate_first_data=1`: transmits the first DATA frame twice;
- `reorder_first_pair=1`: holds the first DATA frame and sends it after the next DATA frame.

These modes validate retransmission, duplicate suppression and the reorder queue.
