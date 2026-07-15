# STCP directional session keys

This stage extends the working X25519 + ChaCha20-Poly1305 transport with:

- HKDF-SHA256 key derivation implemented in `no_std` Rust
- separate client-to-server and server-to-client keys
- both directions start nonce counters from zero because keys are distinct
- exact monotonically increasing receive nonce checks
- TX nonce advances only after successful encryption
- RX nonce advances only after successful authentication
- replay, reordering and skipped nonces are rejected
- module-load crypto selftest
- tampered authentication tag selftest
- wrong-direction key selftest
- key material zeroing on drop

The BSD socket wrappers and loopback carrier are unchanged.

## Build and test

```bash
make LLVM=1 V=1 test
```

The test target loads:

```bash
modprobe libcurve25519
modprobe libchacha20poly1305
```

before inserting `stcp.ko`.

Expected kernel log includes:

```text
stcp: directional crypto selftest passed
```

## Wire protection

Each data frame authenticates its complete 16-byte STCP header as AAD. The wire
payload is:

```text
8-byte big-endian nonce || ciphertext || 16-byte Poly1305 tag
```

A receiver accepts only the exact expected nonce. The expected nonce is updated
only after successful Poly1305 authentication.


## Layered architecture phase

The Rust implementation has been reorganized into explicit `session`, `frame`,
`crypto`, and `carrier` layers. The existing loopback behavior and tests remain
unchanged. The next carrier can be implemented without changing the BSD socket,
session, frame, or crypto interfaces.


## Sequence and ACK protocol phase

The STCP header is now 32 bytes and carries explicit 64-bit sequence and
acknowledgment numbers. Authenticated DATA frames generate ACK control frames.
PING/PONG, CLOSE, and RESET packet types are parsed in the session layer. This
phase still uses strict in-order delivery; retransmission and sliding windows
are intentionally deferred.


## Sliding-window and retransmission phase

This version adds:

- an eight-frame send window,
- cumulative ACK processing,
- retained pending frames,
- per-socket delayed-work retransmission ticks,
- retransmission after approximately 300 ms,
- a five-retry limit,
- duplicate frame suppression,
- automatic loss injection with `drop_first_data=1`.

The standard `make test` target loads the module with one intentionally dropped
DATA frame, so successful tests confirm that retransmission works.


## Large-message sequence validation fix

The receive parser now accounts for encrypted DATA frames already collected
during the same parsing pass. Previously every frame was compared against the
same initial `expected_rx_sequence`, causing the second frame of a multi-frame
message to be rejected as a protocol error.
