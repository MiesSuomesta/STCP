# STCPv2 wire protocol – sequence and ACK phase

## Header

All integer fields are network byte order.

| Offset | Size | Field |
|---:|---:|---|
| 0 | 4 | Magic `STCP` |
| 4 | 1 | Packet type |
| 5 | 1 | Version (`2`) |
| 6 | 2 | Flags |
| 8 | 8 | Payload length |
| 16 | 8 | Sequence number |
| 24 | 8 | Acknowledgment number |

Header size is 32 bytes.

## Packet types

- `1 PublicKey`
- `2 HandshakeDone`
- `3 DataChunk`
- `4 DataChunkEnd`
- `5 Ack`
- `6 Ping`
- `7 Pong`
- `8 Close`
- `9 Reset`

## DATA

Each encrypted DATA frame has a monotonically increasing sequence number. The
AEAD nonce currently equals the frame sequence progression but remains encoded
as a separate 64-bit payload field. The full 32-byte header is authenticated as
associated data.

## ACK

An ACK frame has no payload. Its acknowledgment field contains the highest
successfully authenticated DATA sequence received. ACKs are generated only
after successful AEAD verification.

## Replay and ordering

The receiver accepts only the exact next sequence and nonce. Duplicate,
skipped, or reordered DATA frames move the session to protocol error. Sliding
window reordering support is intentionally deferred to the next phase.

## PING/PONG

PING and PONG have no payload. PONG copies the PING sequence into its sequence
field. They are control frames and do not consume DATA sequence numbers.

## CLOSE/RESET

CLOSE performs orderly EOF. RESET marks a protocol failure and immediately
invalidates the session.
