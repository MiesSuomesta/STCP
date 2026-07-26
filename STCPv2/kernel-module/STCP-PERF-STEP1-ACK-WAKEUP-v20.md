# STCP performance step 1: ACK wakeup coalescing (v20)

Base: user-provided `kernel(4).zip`.

## Change

The v19 correctness fix woke the shared socket wait queue for every UDP ACK,
NACK, PONG, RESET and CLOSE control frame. Under sustained traffic that causes
unnecessary scheduler wakeups and cross-CPU wakeup/IPI traffic even when the
send window was already writable.

v20 moves the TX wakeup decision into cumulative ACK processing and wakes the
socket only when the reliability window transitions from full to writable:

```
pending before ACK >= STCP_SEND_WINDOW
pending after ACK  < STCP_SEND_WINDOW
    -> wake blocked sendmsg()
```

ACKs received while the window is already writable no longer generate a TX
wakeup. Application-data readability and handshake-completion wakeups are
unchanged.

## Files changed

- `common-rust/src/session.rs`
- `common-rust/src/carrier.rs`

## Intentionally unchanged

- wire protocol
- frame size
- send window size
- ACK frequency
- retransmit/RTO logic
- UDP burst size
- encryption

## Validation

The source was structurally inspected and the archive verified. A Rust/kernel
build was not run in the packaging environment because `cargo` and the target
kernel build tree are unavailable there.

Recommended comparison benchmark:

```bash
HOST=192.168.1.199 \
PORT=19002 \
CLIENTS_LIST="1 2 4 8 16" \
bash orchestrate-stcp-udp-tests-fixed.sh
```

Key regression condition: all cases must remain at zero errors, with no return
of the 30-second send stall.
